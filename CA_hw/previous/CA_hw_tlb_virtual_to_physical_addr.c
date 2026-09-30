#include <stdint.h>

// Odd/Even Page Attribute Section (Corresponding to TLBELO0 and TLBELO1)
typedef struct {
    uint64_t ppn  : 36; // PPN: Physical Page Number (36 bits for LoongArch64)
    uint8_t  rplv : 1;  // RPLV: Restricted Privilege Level Enable (1 bit)
    uint8_t  plv  : 2;  // PLV: Privilege Level (2 bits, 0-3)
    uint8_t  mat  : 2;  // MAT: Memory Access Type / Cache Attribute (2 bits)
    uint8_t  nx   : 1;  // NX: No Execute (1 bit)
    uint8_t  nr   : 1;  // NR: No Read (1 bit)
    uint8_t  d    : 1;  // D: Dirty / Write Enable (1 bit)
    uint8_t  v    : 1;  // V: Valid (1 bit)
} TLB_Page_Attr;

// TLB Entry
typedef struct {
    uint64_t vppn : 35; // VPPN: Virtual Page Pair Number (vaddr[47:13] for 48-bit VA)
    uint8_t  ps   : 6;  // PS: Page Size encoding (6 bits, e.g., 12 for 4KB page)
    uint8_t  g    : 1;  // G: Global Flag (1 bit, bypasses ASID check if set)
    uint16_t asid : 10; // ASID: Address Space Identifier (10 bits)
    uint8_t  e    : 1;  // E: Element Existing Flag (1 bit)

    TLB_Page_Attr page0; // Even Page
    TLB_Page_Attr page1; // Odd Page
} LoongArch_TLB_Entry;

#define TLB_SIZE 64
LoongArch_TLB_Entry tlb[TLB_SIZE];

// CSR (part of)
uint16_t CSR_ASID;        // Current Process ASID
uint8_t  CSR_CRMD_PLV;    // Current CPU Privilege Level (0: Kernel, 3: User)
uint64_t CSR_TLBRENTRY;   // TLB Refill Exception Entry Address
uint64_t CSR_BADV;        // Bad Virtual Address Register
uint64_t CSR_TLBEHI_VPPN; // TLB Exception Virtual Page Pair Number

// Access Request Types
typedef enum {
    READ,   // Memory Read Operation
    WRITE,  // Memory Write Operation
    FETCH   // Instruction Fetch Operation
} AccessType;

// Hardware Exception Types
typedef enum {
    EXC_NONE = 0,
    EXC_TLBR,   // TLB Refill Exception
    EXC_PIL,    // Page Invalid Exception for Load
    EXC_PIS,    // Page Invalid Exception for Store
    EXC_PIF,    // Page Invalid Exception for Fetch
    EXC_PME,    // Page Modify Exception (Write Protection)
    EXC_PPI     // Page Privilege Illegal Exception
} ExceptionType;

/**
 * @brief Translates virtual address to physical address following LoongArch spec
 * @param vaddr  64-bit virtual address
 * @param access Access type (READ / WRITE / FETCH)
 * @param paddr  [Output] Translated 64-bit physical address
 * @return ExceptionType Returns EXC_NONE on success, or exception code
 */
ExceptionType translate_loongarch_tlb(uint64_t vaddr, AccessType access, uint64_t *paddr) {
    // Standard 4KB Page (PS = 12):
    // vaddr[63:13] -> VPPN (Virtual Page Pair Number)
    // vaddr[12]    -> Odd/Even Page Selector (0: page0, 1: page1)
    // vaddr[11:0]  -> 4KB Page Offset
    uint64_t vppn = vaddr >> 13;
    uint8_t  odd_page = (vaddr >> 12) & 0x1;
    uint64_t offset = vaddr & 0xFFF;

    int hit_index = -1;

    // 1. Parallel Lookup in TLB Entries
    for (int i = 0; i < TLB_SIZE; i++) {
        // Match condition: Entry Exists (E==1) && VPPN Matches && (Global G==1 || ASID Matches)
        if (tlb[i].e && (tlb[i].vppn == vppn) && (tlb[i].g || (tlb[i].asid == CSR_ASID))) {
            hit_index = i;
            break;
        }
    }

    // TLB Refill Exception
    if (hit_index == -1) {
        CSR_BADV = vaddr;
        CSR_TLBEHI_VPPN = vppn;
        return EXC_TLBR; // Jump to CSR.TLBRENTRY
    }

    // 2. Extract Attribute Section for Target Sub-page
    LoongArch_TLB_Entry entry = tlb[hit_index];
    TLB_Page_Attr attr = odd_page ? entry.page1 : entry.page0;

    // Page Invalid Exceptions
    if (!attr.v) { // V == 0 indicates page is invalid
        CSR_BADV = vaddr;
        CSR_TLBEHI_VPPN = vppn;
        if (access == READ)  return EXC_PIL;
        if (access == WRITE) return EXC_PIS;
        if (access == FETCH) return EXC_PIF;
    }

    // Privilege Level Check
    if (attr.rplv == 1) {
        if (CSR_CRMD_PLV > attr.plv) {
            CSR_BADV = vaddr;
            return EXC_PPI; // Page Privilege Illegal
        }
    }

    // Access Rights Control (NR / NX / D)
    if (access == READ && attr.nr) { // NR == 1: No Read
        CSR_BADV = vaddr;
        return EXC_PIL;
    }

    if (access == FETCH && attr.nx) { // NX == 1: No Execute
        CSR_BADV = vaddr;
        return EXC_PIF;
    }

    if (access == WRITE && !attr.d) { // D == 0: Write Protected / Clean Page
        CSR_BADV = vaddr;
        return EXC_PME; // Page Modify Exception
    }

    // 3. Successful Address Translation
    *paddr = ((uint64_t)attr.ppn << 12) | offset;
    return EXC_NONE;
}
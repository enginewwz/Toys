#include <stdint.h>
#include <stdbool.h>

#define PAGE_SHIFT        12                      // page size 4KB (2^12)
#define PAGE_MASK         ((1ULL << PAGE_SHIFT) - 1)
#define SEGMENT_SHIFT     22                      // segment size 4MB (2^10 pages)
#define SEGMENT_MASK      ((1ULL << (SEGMENT_SHIFT - PAGE_SHIFT)) - 1)

typedef uint32_t virt_addr_t;
typedef uint64_t phys_addr_t;

// 表项定义
typedef struct {
    uint32_t page_table_base_pfn; // 对应页表的物理页框号
    uint32_t page_count;          // 该段包含的最大页数（段界限检查）
    bool valid;                   // 段表项有效位
    uint8_t dpl;                  // 段保护特权级 (Descriptor Privilege Level)
} seg_table_entry_t;

typedef struct {
    uint32_t pfn;                 // 物理页框号 (Physical Frame Number)
    bool present;                 // 存在位 (1: 在内存中, 0: 缺页)
    bool readable;                // 读权限
    bool writable;                // 写权限
    bool executable;              // 执行权限
    uint8_t dpl;                  // 页保护特权级
} page_table_entry_t;

seg_table_entry_t* GDTR_base;     // 段表基地址寄存器
uint32_t GDTR_limit;              // 段表长度限制
uint8_t CPL;                      // 当前 CPU 特权级 (Current Privilege Level)

// 硬件异常枚举
typedef enum {
    MMU_SUCCESS = 0,
    FAULT_SEGMENT_OUT_OF_BOUNDS,  // 段号越界
    FAULT_SEGMENT_NOT_PRESENT,    // 段描述符无效/段不存在
    FAULT_SEGMENT_PROTECTION,     // 段权限违例
    FAULT_PAGE_OUT_OF_BOUNDS,     // 页号超出段界限
    FAULT_PAGE_FAULT,             // 缺页异常
    FAULT_PAGE_PROTECTION         // 页权限违例
} mmu_fault_t;

// 转换函数
mmu_fault_t translate_virtual_to_physical(
    virt_addr_t vaddr, 
    uint8_t access_type,          // 0: READ, 1: WRITE, 2: EXECUTE
    phys_addr_t* paddr_out
) {
    // 提取虚拟地址字段
    uint32_t seg_num  = vaddr >> SEGMENT_SHIFT;                          // 提取段号
    uint32_t page_num = (vaddr >> PAGE_SHIFT) & SEGMENT_MASK;            // 提取段内页号
    uint32_t offset   = vaddr & PAGE_MASK;                               // 提取页内偏移

    // 查段表
    // 1. 检查段号是否超出段表界限
    if (seg_num >= GDTR_limit) {
        return FAULT_SEGMENT_OUT_OF_BOUNDS;
    }

    seg_table_entry_t seg_entry = GDTR_base[seg_num];

    // 2. 检查段表项有效性
    if (!seg_entry.valid) {
        return FAULT_SEGMENT_NOT_PRESENT;
    }

    // 3. 检查段级特权检查 (CPL 权限不能低于 DPL)
    if (CPL > seg_entry.dpl) {
        return FAULT_SEGMENT_PROTECTION;
    }

    // 4. 检查页号是否超出该段声明的最大页数
    if (page_num >= seg_entry.page_count) {
        return FAULT_PAGE_OUT_OF_BOUNDS;
    }

    // 查页表
    phys_addr_t page_table_phys_base = (phys_addr_t)seg_entry.page_table_base_pfn << PAGE_SHIFT;
    page_table_entry_t* page_table = (page_table_entry_t*)page_table_phys_base;

    page_table_entry_t page_entry = page_table[page_num];

    // 1. 检查页是否存在
    if (!page_entry.present) {
        // 触发缺页异常
        return FAULT_PAGE_FAULT; 
    }

    // 2. 检查页级访问权限
    if (access_type == 1 && !page_entry.writable) {
        return FAULT_PAGE_PROTECTION; // 只读页尝试写入
    }
    if (access_type == 2 && !page_entry.executable) {
        return FAULT_PAGE_PROTECTION; // 禁止执行页尝试执行代码
    }
    if (CPL > page_entry.dpl) {
        return FAULT_PAGE_PROTECTION; // 越权访问页
    }

    // 合成最终物理地址
    *paddr_out = ((phys_addr_t)page_entry.pfn << PAGE_SHIFT) | offset;

    return MMU_SUCCESS;
}

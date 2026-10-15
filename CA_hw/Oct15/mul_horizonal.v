module mul_horizonal (
    input  wire [31:0] src1,
    input  wire [31:0] src2,

    output wire [63:0] result
);
    // booth
    wire [63:0] L0 [16:0];
    wire [32:0] booth_src1 = {src1, 1'b0};
    wire [63:0] booth_src2 = {{33{src2[31]}}, src2[30:0]};
    wire [31:0] booth_neg;   // corrections only reach bit 2*15+1 = 31

    genvar j;

    generate
        for (j = 0; j < 16; j ++) begin : booth_mul
            wire [2:0] y = booth_src1[2 * j + 2 : 2 * j];
            wire sel_1 = y[1] ^ y[0];
            wire sel_2 = y[2] & ~y[1] & ~y[0] | ~y[2] & y[1] & y[0];

            wire [63:0] booth_src2_neg_sel = y[2] ? ~ booth_src2 : booth_src2;
            assign L0[j] = sel_2 ? booth_src2_neg_sel << (2 * j + 1) : sel_1 ? booth_src2_neg_sel << (2 * j) : 64'b0;
            assign booth_neg[2 * j + 1] = sel_2 & y[2];
            assign booth_neg[2 * j] = sel_1 & y[2];
        end
    endgenerate

    assign L0[16] = {32'b0, booth_neg};

    // 17 -> 12 -> 8 -> 6 -> 4 -> 3 -> 2
    wire [63:0] L1 [11:0];
    wire [63:0] L2 [ 7:0];
    wire [63:0] L3 [ 5:0];
    wire [63:0] L4 [ 3:0];
    wire [63:0] L5 [ 2:0];

    wire [63:0] s, c;

    generate
        for (j = 0; j < 5; j ++) begin : l0_to_l1
            csa64 u_csa (
                .a      (L0[j * 3]      ),
                .b      (L0[j * 3 + 1]  ),
                .cin    (L0[j * 3 + 2]  ),

                .cout   (L1[j * 2]      ),
                .s      (L1[j * 2 + 1]  )
            );
        end
    endgenerate

    assign L1[10] = L0[15];
    assign L1[11] = L0[16];

    generate
        for (j = 0; j < 4; j ++) begin : l1_to_l2
            csa64 u_csa (
                .a      (L1[j * 3]      ),
                .b      (L1[j * 3 + 1]  ),
                .cin    (L1[j * 3 + 2]  ),

                .cout   (L2[j * 2]      ),
                .s      (L2[j * 2 + 1]  )
            );
        end
    endgenerate

    generate
        for (j = 0; j < 2; j ++) begin : l2_to_l3
            csa64 u_csa (
                .a      (L2[j * 3]      ),
                .b      (L2[j * 3 + 1]  ),
                .cin    (L2[j * 3 + 2]  ),

                .cout   (L3[j * 2]      ),
                .s      (L3[j * 2 + 1]  )
            );
        end
    endgenerate

    assign L3[4] = L2[6];
    assign L3[5] = L2[7];

    generate
        for (j = 0; j < 2; j ++) begin : l3_to_l4
            csa64 u_csa (
                .a      (L3[j * 3]      ),
                .b      (L3[j * 3 + 1]  ),
                .cin    (L3[j * 3 + 2]  ),

                .cout   (L4[j * 2]      ),
                .s      (L4[j * 2 + 1]  )
            );
        end
    endgenerate

    csa64 u_csa_level_4 (
        .a      (L4[0]  ),
        .b      (L4[1]  ),
        .cin    (L4[2]  ),

        .cout   (L5[0]  ),
        .s      (L5[1]  )
    );

    assign L5[2] = L4[3];

    csa64 u_csa_level_5 (
        .a      (L5[0]  ),
        .b      (L5[1]  ),
        .cin    (L5[2]  ),

        .cout   (c      ),
        .s      (s      )
    );

    assign result = s + c;

endmodule

module csa64 (
    input  wire [63:0] a,
    input  wire [63:0] b,
    input  wire [63:0] cin,

    output wire [63:0] cout,
    output wire [63:0] s 
);
    assign s = a ^ b ^ cin;
    assign cout = (a & b | a & cin | b & cin) << 1'b1;
endmodule
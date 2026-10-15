module mul_vertical (
    input  wire [31:0] src1,
    input  wire [31:0] src2,

    output wire [63:0] result
);
    // booth
    wire [63:0] booth_result [15:0];
    wire [32:0] booth_src1 = {src1, 1'b0};
    wire [63:0] booth_src2 = {{33{src2[31]}}, src2[30:0]};
    wire [15:0] booth_neg;

    genvar j, k;

    generate
        for (j = 0; j < 16; j ++) begin : booth_mul
            wire [2:0] y = booth_src1[2 * j + 2 : 2 * j];
            wire sel_1 = y[1] ^ y[0];
            wire sel_2 = y[2] & ~y[1] & ~y[0] | ~y[2] & y[1] & y[0];

            wire [63:0] booth_src2_shifted = sel_2 ? booth_src2 << (2 * j + 1) : sel_1 ? booth_src2 << (2 * j) : 64'b0;
            assign booth_result[j] = y[2] ? ~ booth_src2_shifted : booth_src2_shifted;
            assign booth_neg[j] = y[2];
        end
    endgenerate

    wire [63:0] c, s;
    wire [13:0] cin  [64:0];
    wire [13:0] cout [64:0];
    wire [15:0] csa_src [63:0];

    generate
        for (j = 0; j < 64; j ++) begin : csa_src_gen
            for (k = 0; k < 16; k ++) begin
                assign csa_src [j][k] = booth_result [k][j];
            end    
        end
    endgenerate

    assign cin[0] = booth_neg [13:0];

    generate
        for (j = 0; j < 64; j ++) begin : csa_tree
            csa_vertical u_csa (
                .src    (csa_src[j] ),
                .cin    (cin[j]     ),

                .cout   (cout[j]    ),
                .c      (c[j]       ),
                .s      (s[j]       )
            );
            assign cin[j + 1] = cout[j];
        end
    endgenerate

    assign result = {c[62:0], booth_neg[14]} + s + {63'b0, booth_neg[15]};

endmodule

module csa_vertical (
    input  wire [15:0] src,
    input  wire [13:0] cin,

    output wire [13:0] cout,
    output wire c,
    output wire s
);
    genvar i;

    wire [15:0] level_0_src = src;
    wire [ 4:0] level_0_sum;

    generate
        for (i = 0; i < 5 ; i ++) begin : level_0_csa
            full_adder u_fa (
                .a      (level_0_src[i * 3      ]),
                .b      (level_0_src[i * 3 + 1  ]),
                .cin    (level_0_src[i * 3 + 2  ]),

                .cout   (cout[i]),
                .s      (level_0_sum[i])
            );
        end
    endgenerate

    wire [10:0] level_1_src = {level_0_sum, level_0_src[15], cin[4:0]};
    wire [ 2:0] level_1_sum;

    generate
        for (i = 0; i < 3; i ++) begin : level_1_csa
            full_adder u_fa (
                .a      (level_1_src[i * 3      ]),
                .b      (level_1_src[i * 3 + 1  ]),
                .cin    (level_1_src[i * 3 + 2  ]),

                .cout   (cout[i + 5]),
                .s      (level_1_sum[i])
            );
        end
    endgenerate

    wire [ 7:0] level_2_src = {level_1_sum, level_1_src[10:9], cin[7:5]};
    wire [ 2:0] level_2_sum;

    generate
        for (i = 0; i < 2; i ++) begin : level_2_csa
            full_adder u_fa (
                .a      (level_2_src[i * 3      ]),
                .b      (level_2_src[i * 3 + 1  ]),
                .cin    (level_2_src[i * 3 + 2  ]),

                .cout   (cout[i + 8]),
                .s      (level_2_sum[i])
            );
        end
    endgenerate

    half_adder u_ha (
        .a      (level_2_src[7]),
        .b      (level_2_src[6]),

        .c      (cout[10]),
        .s      (level_2_sum[2])
    );

    wire [ 5:0] level_3_src = {level_2_sum, cin[10:8]};
    wire [ 1:0] level_3_sum;

    generate
        for (i = 0; i < 2; i ++) begin : level_3_csa
            full_adder u_fa (
                .a      (level_3_src[i * 3      ]),
                .b      (level_3_src[i * 3 + 1  ]),
                .cin    (level_3_src[i * 3 + 2  ]),

                .cout   (cout[i + 11]),
                .s      (level_3_sum[i])
            );
        end
    endgenerate

    wire [ 3:0] level_4_src = {level_3_sum, cin[12:11]};
    wire level_4_sum;

    full_adder u_fa_level_4 (
        .a      (level_4_src[0]),
        .b      (level_4_src[1]),
        .cin    (level_4_src[2]),

        .cout   (cout[13]),
        .s      (level_4_sum)
    );

    wire [ 2:0] level_5_src = {level_4_sum, level_4_src[3], cin[13]};

    full_adder u_fa_level_5 (
        .a      (level_5_src[0]),
        .b      (level_5_src[1]),
        .cin    (level_5_src[2]),

        .cout   (c),
        .s      (s)
    );

endmodule

module full_adder (
    input  wire a,
    input  wire b,
    input  wire cin,

    output wire cout,
    output wire s
);
    assign s = a ^ b ^ cin;
    assign cout = a & b | a & cin | b & cin;
endmodule

module half_adder (
    input  wire a,
    input  wire b,

    output wire c,
    output wire s
);
    assign s = a ^ b;
    assign c = a & b;
    
endmodule
// -----------------------------------------------------------------------------
// tb_mul.v -- self-checking testbench for the two 32x32 signed multipliers
//             mul_horizonal (CSA tree, "horizontal")  and  mul_vertical
//
//   run:  iverilog -g2012 -o tb_mul.vvp tb_mul.v mul_horizonal.v mul_vertical.v
//         vvp tb_mul.vvp
//         vvp tb_mul.vvp +vcd      (also writes tb_mul.vcd)
// -----------------------------------------------------------------------------
`timescale 1ns / 1ps

module tb_mul;

    reg  [31:0] a, b;
    wire [63:0] r_hor;
    wire [63:0] r_ver;

    // ---- golden reference: sign-extend both operands to 64 bit, then multiply.
    //      (a*b alone would be truncated to 32 bit by Verilog's width rules)
    wire signed [63:0] a_se = {{32{a[31]}}, a};
    wire signed [63:0] b_se = {{32{b[31]}}, b};
    wire signed [63:0] expected = a_se * b_se;

    mul_horizonal u_hor (.src1(a), .src2(b), .result(r_hor));
    mul_vertical  u_ver (.src1(a), .src2(b), .result(r_ver));

    integer tests, err_h, err_v;
    integer i, seed;
    reg [31:0] da, db;                  // directed-case operands (swept below)

    task check;
        input [31:0] ta;
        input [31:0] tbb;
        begin
            a = ta;  b = tbb;
            #1;                          // let the combinational logic settle
            tests = tests + 1;

            if (r_hor !== expected[63:0]) begin
                err_h = err_h + 1;
                if (err_h <= 8)
                    $display("  [HOR FAIL] a=%0d (0x%08h)  b=%0d (0x%08h)  exp=0x%016h  got=0x%016h",
                             $signed(ta), ta, $signed(tbb), tbb, expected, r_hor);
            end

            if (r_ver !== expected[63:0]) begin
                err_v = err_v + 1;
                if (err_v <= 8)
                    $display("  [VER FAIL] a=%0d (0x%08h)  b=%0d (0x%08h)  exp=0x%016h  got=0x%016h",
                             $signed(ta), ta, $signed(tbb), tbb, expected, r_ver);
            end
        end
    endtask

    initial begin
        a = 32'd0;  b = 32'd0;
        tests = 0;  err_h = 0;  err_v = 0;  seed = 32'h1234_5678;

        if ($test$plusargs("vcd")) begin
            $dumpfile("tb_mul.vcd");
            $dumpvars(0, tb_mul);
        end

        $display("== 1. directed corner cases ==");
        //                  a            b
        check(32'h0000_0000, 32'h0000_0000);   // 0 * 0
        check(32'h0000_0000, 32'hFFFF_FFFF);   // 0 * -1
        check(32'h0000_0001, 32'h0000_0001);   // 1 * 1
        check(32'hFFFF_FFFF, 32'hFFFF_FFFF);   // -1 * -1
        check(32'hFFFF_FFFF, 32'h8000_0000);   // -1 * INT_MIN
        check(32'h7FFF_FFFF, 32'h7FFF_FFFF);   // (2^31-1)^2
        check(32'h8000_0000, 32'h8000_0000);   // INT_MIN^2 = 2^62
        check(32'h8000_0000, 32'h7FFF_FFFF);   // INT_MIN * (2^31-1)
        check(32'h8000_0000, 32'h0000_0001);   // INT_MIN * 1
        check(32'h0000_0006, 32'h1234_5678);   // small +, Booth digit +2
        check(32'hFFFF_FFFA, 32'h1234_5678);   // small -, Booth digit -2
        check(32'h0000_0007, 32'h0000_000B);   // 7 * 11
        check(32'hFFFF_FFF9, 32'h0000_000B);   // -7 * 11
        check(32'h0000_0002, 32'hFFFF_FFFD);   // 2 * -3
        check(32'hFFFF_FFFE, 32'h0000_0003);   // -2 * 3
        check(32'h5555_5555, 32'h5555_5555);   // 0101.. pattern  (digits +1)
        check(32'hAAAA_AAAA, 32'h5555_5555);   // 1010.. pattern  (digits -1)
        check(32'h3333_3333, 32'h9999_9999);   // 0011../1001.. patterns
        check(32'hCCCC_CCCC, 32'h3333_3333);
        check(32'h0000_FFFF, 32'hFFFF_0000);
        check(32'h0001_0000, 32'h0001_0000);   // 2^16 * 2^16 = 2^32
        check(32'h0000_0001, 32'h8000_0000);
        check(32'h7FFF_FFFF, 32'h8000_0000);
        check(32'h1234_5678, 32'h9ABC_DEF0);
        check(32'hDEAD_BEEF, 32'hCAFE_BABE);

        $display("== 2. exhaustive small operands: -64..63 x -64..63 ==");
        for (i = 0; i < 128; i = i + 1)
            for (integer k = 0; k < 128; k = k + 1)
                check(i - 64, k - 64);

        $display("== 3. random operands ==");
        for (i = 0; i < 5000; i = i + 1) begin
            da = $random(seed);
            db = $random(seed);
            check(da, db);
        end

        $display("== 4. random, one operand kept small (Booth digit coverage) ==");
        for (i = 0; i < 5000; i = i + 1) begin
            da = $random(seed);
            db = $random(seed) & 32'h0000_00FF;        //  0 .. 255
            check(da, db);
            db = -($random(seed) & 32'h0000_00FF);     // -255 .. 0
            check(da, db);
        end

        $display("-----------------------------------------------------------");
        $display("total tests   : %0d", tests);
        $display("mul_horizonal : %0d mismatch(es)", err_h);
        $display("mul_vertical  : %0d mismatch(es)", err_v);
        if (err_h == 0 && err_v == 0)
            $display("RESULT: PASS");
        else
            $display("RESULT: FAIL");
        $display("-----------------------------------------------------------");
        $finish;
    end

endmodule

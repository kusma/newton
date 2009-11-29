.bss
.align 2
dist_remap:
.space 1024

.section .iwram,"ax",%progbits

.global infzoom_draw
.type   infzoom_draw, %function

/* 2x unrolling required for 30fps with music in lamer mode ;) */
#define UNROLL

.align 2
infzoom_draw:
	@ Function supports interworking.
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	
	stmfd sp!, {r4-r12,r14}
	/* unused registers: r13/sp, (r14/lr) */
	
	/* load pointers */
	ldr r12, =zoom_lut
	ldr r10, =dist_remap
	
	mov r7, r2, asr #0 /* discard bottom 8 bits from zoom */
	add r4, r12, #512  /* fetch address of the end of zoom_lut */
	
	/* setup dist_remap */
.update_dist_remap:
	ldrh  r2, [r12], #2   /* load entry from zoom_lut */
	umull r3, r11, r2, r7 /* multiply with zoom */
	mov r3, r3, lsr #24   /* fetch bottom 8 bits */
	orr r3, r11, lsl #8   /* fetch top 24 bit */
	str r3, [r10], #4     /* store remap entry */
	cmp r12, r4           /* check if we're at the end of zoom_lut */
	bne .update_dist_remap
	
	sub r10, r10, #1024 /* get back to the start of dist_remap */

	/* load pointers */
	ldr r11, =zoom_table
	ldr r8,  =zoom_table+38400 /* load stop address (we're comparing on zoom_table) */

	mov r12, #255

.yloop:
	add r4, r0, #238 /* setup pointer to the rightmost pixel on the scanline */
#ifdef UNROLL
	mov r14, #30 /* iteration count */
#else
	mov r14, #60 /* iteration count */
#endif

.xloop:
#ifdef UNROLL
	ldmia r11!, {r5-r6}
#else
	ldr   r5, [r11], #4
#endif
	
	/* unroll #1 */
	
	/* make uv2 */
	mov r9, r5, lsr #24       /* mask out dist */
	ldr r7, [r10, r9, asl #2] /* remap dist */
	and r2, r12, r5, asr #16  /* mask out deg */
	orr r2, r2, r7, asl #8    /* OR togeither dist and deg */

	/* make uv1 */
	and r3, r12, r5, asr #8   /* mask out dist */
	ldr r7, [r10, r3, asl #2] /* remap dist */
	and r5, r5, #255          /* mask out deg */
	orr r5, r5, r7, asl #8    /* OR togeither dist and deg */
	
	/* load leftmost texels (uv1 and uv2) */
	ldrb r9, [r1, r2]
	ldrb r3, [r1, r5]

	/* store leftmost pixels */
	orr  r3, r3, r9, asl #8 /* OR togeither pixels */
	strh r3, [r0], #2   /* store */

	/* flip all bits = 1 - x + 1, ALMOST what we want - let's use it ;) */
	eor r2, r2, #255
	eor r5, r5, #255
	
	/* load rightmost texels (uv1 and uv2) */
	ldrb r3, [r1, r2]
	ldrb r9, [r1, r5]

	/* store rightmost pixels */
	orr  r3, r3, r9, asl #8  /* OR togeither pixels */
	strh r3, [r4], #-2   /* store */
	
	
#ifdef UNROLL
	/* unroll #2 */
	
	/* make uv2 */
	mov r9, r6, lsr #24      /* mask out dist */
	ldr r7, [r10, r9, asl #2] /* remap dist */
	and r2, r12, r6, asr #16  /* mask out deg */
	orr r2, r2, r7, asl #8   /* OR togeither dist and deg */

	/* make uv1 */
	and r3, r12, r6, asr #8   /* mask out dist */
	ldr r7, [r10, r3, asl #2] /* remap dist */
	and r6, r6, #255         /* mask out deg */
	orr r6, r6, r7, asl #8   /* OR togeither dist and deg */
	
	/* load leftmost texels (uv1 and uv2) */
	ldrb r9, [r1, r2]
	ldrb r3, [r1, r6]

	/* store leftmost pixels */
	orr  r3, r3, r9, asl #8 /* OR togeither pixels */
	strh r3, [r0], #2   /* store */

	/* flip all bits = 1 - x + 1, ALMOST what we want - let's use it ;) */
	eor r2, r2, #255
	eor r6, r6, #255
	
	/* load rightmost texels (uv1 and uv2) */
	ldrb r3, [r1, r2]
	ldrb r9, [r1, r6]

	/* store rightmost pixels */
	orr  r3, r3, r9, asl #8  /* OR togeither pixels */
	strh r3, [r4], #-2   /* store */
#endif

	/* loop, brotha! */
	subs r14, r14, #1
	bne  .xloop
	
	add r0, r0, #120 /* update framebuffer pointer */
	cmp r11, r8 /* check for termination (r8 contains end of LUT) */
	bne .yloop
	
	/* hasta lavista, baby */
	ldmfd sp!, {r4-r12,r14}
	bx lr

.align 2
.size  infzoom_draw, .-infzoom_draw

.section .rodata
.data
.align 1
.global zoom_lut

.type zoom_lut, %object
.size zoom_lut, 512
zoom_lut:
	.short 0
	.short 4096
	.short 5792
	.short 7094
	.short 8192
	.short 9158
	.short 10033
	.short 10836
	.short 11585
	.short 12288
	.short 12952
	.short 13584
	.short 14188
	.short 14768
	.short 15325
	.short 15863
	.short 16384
	.short 16888
	.short 17377
	.short 17854
	.short 18317
	.short 18770
	.short 19211
	.short 19643
	.short 20066
	.short 20480
	.short 20885
	.short 21283
	.short 21673
	.short 22057
	.short 22434
	.short 22805
	.short 23170
	.short 23529
	.short 23883
	.short 24232
	.short 24576
	.short 24914
	.short 25249
	.short 25579
	.short 25905
	.short 26227
	.short 26545
	.short 26859
	.short 27169
	.short 27476
	.short 27780
	.short 28080
	.short 28377
	.short 28672
	.short 28963
	.short 29251
	.short 29536
	.short 29819
	.short 30099
	.short 30376
	.short 30651
	.short 30924
	.short 31194
	.short 31461
	.short 31727
	.short 31990
	.short 32251
	.short 32510
	.short -32768
	.short -32513
	.short -32260
	.short -32009
	.short -31760
	.short -31513
	.short -31267
	.short -31023
	.short -30781
	.short -30540
	.short -30301
	.short -30064
	.short -29828
	.short -29594
	.short -29362
	.short -29130
	.short -28901
	.short -28672
	.short -28446
	.short -28220
	.short -27996
	.short -27773
	.short -27552
	.short -27332
	.short -27113
	.short -26895
	.short -26678
	.short -26463
	.short -26249
	.short -26036
	.short -25824
	.short -25614
	.short -25404
	.short -25196
	.short -24988
	.short -24782
	.short -24576
	.short -24372
	.short -24169
	.short -23967
	.short -23765
	.short -23565
	.short -23366
	.short -23167
	.short -22970
	.short -22773
	.short -22577
	.short -22382
	.short -22189
	.short -21995
	.short -21803
	.short -21612
	.short -21421
	.short -21231
	.short -21043
	.short -20854
	.short -20667
	.short -20480
	.short -20295
	.short -20110
	.short -19925
	.short -19742
	.short -19559
	.short -19377
	.short -19196
	.short -19015
	.short -18835
	.short -18656
	.short -18477
	.short -18299
	.short -18122
	.short -17945
	.short -17769
	.short -17594
	.short -17419
	.short -17245
	.short -17072
	.short -16899
	.short -16727
	.short -16555
	.short -16384
	.short -16214
	.short -16044
	.short -15875
	.short -15707
	.short -15538
	.short -15371
	.short -15204
	.short -15038
	.short -14872
	.short -14706
	.short -14542
	.short -14377
	.short -14214
	.short -14051
	.short -13888
	.short -13726
	.short -13564
	.short -13403
	.short -13242
	.short -13082
	.short -12922
	.short -12763
	.short -12605
	.short -12446
	.short -12288
	.short -12131
	.short -11974
	.short -11818
	.short -11662
	.short -11507
	.short -11352
	.short -11197
	.short -11043
	.short -10889
	.short -10736
	.short -10583
	.short -10430
	.short -10278
	.short -10127
	.short -9976
	.short -9825
	.short -9675
	.short -9525
	.short -9375
	.short -9226
	.short -9077
	.short -8929
	.short -8781
	.short -8633
	.short -8486
	.short -8339
	.short -8192
	.short -8046
	.short -7901
	.short -7755
	.short -7610
	.short -7466
	.short -7321
	.short -7177
	.short -7034
	.short -6891
	.short -6748
	.short -6605
	.short -6463
	.short -6321
	.short -6180
	.short -6039
	.short -5898
	.short -5757
	.short -5617
	.short -5477
	.short -5338
	.short -5199
	.short -5060
	.short -4921
	.short -4783
	.short -4645
	.short -4507
	.short -4370
	.short -4233
	.short -4096
	.short -3960
	.short -3824
	.short -3688
	.short -3553
	.short -3418
	.short -3283
	.short -3148
	.short -3014
	.short -2880
	.short -2746
	.short -2613
	.short -2479
	.short -2346
	.short -2214
	.short -2082
	.short -1949
	.short -1818
	.short -1686
	.short -1555
	.short -1424
	.short -1293
	.short -1163
	.short -1033
	.short -903
	.short -773
	.short -644
	.short -515
	.short -386
	.short -257
	.short -129


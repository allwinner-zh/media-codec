.code 32
	.fpu neon
	.align 4
	.globl	ImgRGBA2YUV420SP_neon
	.func
ImgRGBA2YUV420SP_neon:
    stmfd                       sp!, {r4-r11,lr}  
 
	LDR				r4, [r2, #4]
	LDR				lr, [r2]
	mov				r2, lr
	
	mul				r6, r2, r3							

	LDR				r7, [r1, #4]
	LDR				lr, [r1]
	mov				r1, lr
							
	mov				lr, r4, lsl #2							
	mov       r8, r2, lsl #2	
	mov       r11, r4, lsl #3	
	sub				r6, r11, r8
			
	add				r9, lr, r0
	add				r10,r2, r1	
	mov				r11, #32						
	
								
       
  mov				r5, #66	                     
	vdup.8		d0, r5                                
	                                                 
	mov			r5, #129	                       
	vdup.8		d1, r5                                
                                                   
	mov			r5, #25	                       
	vdup.8		d2, r5                                
                                                   
	mov			r5, #38	                        
	vdup.8		d3, r5                                
                                                   
	mov			r5, #74	                       
	vdup.8		d4, r5                                
                                                   
	mov			r5, #112	                               
	vdup.8		d5, r5                                
                                                   
	mov			r5, #94	                      
	vdup.8		d6, r5                                
                                                   
	mov			r5, #18	                        
	vdup.8		d7, r5                                
                                                   
	mov			r5, #16	                      
	vdup.8		d30, r5                                
                                                   
	mov			r5, #128                     
	vdup.8		d31, r5             						               
                                    
  mov       r5, r2, lsr #3					
  
loop_row:
loop_col:  
  
  subs            r5, r5, #1               
        vld4.u8         {d8,d9,d10,d11}, [r0], r11		
        vld4.u8         {d12,d13,d14,d15}, [r9], r11	

        PLD				[r0,#64]  
        PLD				[r9,#64]   
                                                   
		vmull.u8		q8,	d10,	d0   								
		vmlal.u8		q8,	d9,	d1                   
		vmlal.u8		q8,	d8,	d2 
		
		vmull.u8		q9,	d8,	d5                   
		vmlsl.u8		q9,	d9,	d4 
		vmlsl.u8		q9, d10, d3
		
		vmull.u8		q10,	d10,	d5                   
		vmlsl.u8		q10,	d9,	d6                   
		vmlsl.u8		q10,	d8,	d7                   
		
		vrshrn.i16    d8, q8, #8										
    vrshrn.i16    d9, q9, #8 									
    vrshrn.i16    d10, q10, #8 									
     
    
    vadd.i8				d27,	d8, d30  								
    vadd.i8				d28,	d9, d31									
    vadd.i8				d29,	d10, d31									
    
    
    vmull.u8		q8,	d14,	d0   							
		vmlal.u8		q8,	d13,	d1                   
		vmlal.u8		q8,	d12,	d2   
		
		vmull.u8		q9,	d12, d5                  
		vmlsl.u8		q9,	d13, d4  
		vmlsl.u8		q9, d14,  d3
		
		vmull.u8		q10,	d14,	d5                  
		vmlsl.u8		q10,	d13,	d6                    
		vmlsl.u8		q10,	d12,	d7                    
		
		vrshrn.i16    d8, q8,  #8
    vrshrn.i16    d9, q9,  #8 
    vrshrn.i16    d10, q10, #8 
     
    
    vadd.i8				d24,	d8, d30   									
    vadd.i8				d25,	d9, d31											
    vadd.i8				d26,	d10, d31										
    
    vaddl.u8				q4, d28, d25										
    vaddl.u8				q5, d29, d26										
    
    vpaddl.u16			q6, q4
    vpaddl.u16      q7, q5									
    
    vrshrn.i32			d8, q6, #2										
    vrshrn.i32			d9, q7,#2		
    
    vshl.i16				d10, d9, #8
    vadd.i16        d11, d8, d10
    										
                     
                                                
		vst1.8			{d27},[r1]!											
		vst1.8			{d24}, [r10]!											
		
		vst1.8			{d11},[r7]!											
												
			  		    
	 bgt         loop_col
        
   subs        r3, r3, #2 
   mov         r5, r2, lsr #3
   add				 r0, r0, r6													
   add				 r9, r9, r6
   mov				 r1, r10
   add				 r10,r10,r2											
      
   bgt             loop_row                
        
   ldmfd                       sp!, {r4-r11,lr}    
   bx              lr                              
                                                   
    .endfunc                                       
		.end      	  		                           
		.end      	  		
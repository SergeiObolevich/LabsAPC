.386P
.MODEL  LARGE
;��������� ������
S_DESC  struc                                   ;��������� ����������� �����������
    LIMIT       dw 0                            ;����� �������� (15:0)    
    BASE_L      dw 0                            ;����� ����, ������� ����� (15:0)
    BASE_M      db 0                            ;����� ����, ������� ����� (23:16)
    ACCESS      db 0                            ;���� �������
    ATTRIBS     db 0                            ;����� �������� (19:16) � ��������
    BASE_H      db 0                            ;����� ����, ������� �����
S_DESC  ends        
I_DESC  struc                                   ;��������� ����������� ������� ����������
    OFFS_L      dw 0                            ;����� ����������� (0:15)
    SEL         dw 0                            ;�������� ����, ����������� ��� �����������
    PARAM_CNT   db 0                            ;���������
    ACCESS      db 0                            ;������� �������
    OFFS_H      dw 0                            ;����� ����������� (31:16)
I_DESC  ends        
R_IDTR  struc                                   ;��������� IDTR
    LIMIT       dw 0                            
    IDT_L       dw 0                            ;�������� ���� (0-15)
    IDT_H       dw 0                            ;�������� ���� (31-16)
R_IDTR  ends
;����� ������� ������� ���������
ACS_PRESENT     EQU 10000000B                   ;PXXXXXXX - ��� �����������, ������� ������������ � ����������� ������
ACS_CSEG        EQU 00011000B                   ;XXXXIXXX - ��� ��������, ��� ������ = 0, ��� ���� 1
ACS_DSEG        EQU 00010000B                   ;XXXSXXXX - ��� ��������, ������ ������ �������(��������� ������� ����� ���� �� ��������)
ACS_READ        EQU 00000010B                   ;XXXXXXRX - ��� ������, ����������� ������ �� ������� ��������
ACS_WRITE       EQU 00000010B                   ;XXXXXXWX - ��� ������, ��� �������� ������ ��������� ������
ACS_CODE        =   ACS_PRESENT or ACS_CSEG     ;AR �������� ����
ACS_DATA =  ACS_PRESENT or ACS_DSEG or ACS_WRITE;AR �������� ������
ACS_STACK=  ACS_PRESENT or ACS_DSEG or ACS_WRITE;AR �������� �����
ACS_INT_GATE    EQU 00001110B
ACS_TRAP_GATE   EQU 00001111B                   ;XXXXSICR - �������, ����������� ������� ����, �������� ��� ������
ACS_IDT         EQU ACS_DATA                    ;AR ������� IDT    
ACS_INT         EQU ACS_PRESENT or ACS_INT_GATE
ACS_TRAP        EQU ACS_PRESENT or ACS_TRAP_GATE
ACS_DPL_3       EQU 01100000B                   ;X<DPL,DPL>XXXXX - ���������� �������, ������ ����� �������� ����� ���
;������� ���� ��������� ������       
CODE_RM segment para use16
CODE_RM_BEGIN   = $
    assume cs:CODE_RM,DS:DATA,ES:DATA           ;������������� ��������� ��� ���������������
START:
    mov ax,DATA                                 ;������������� ���������� ���������
    mov ds,ax                                   
    mov es,ax                          
    lea dx,MSG_ENTER
    mov ah,9h
    int 21h
    call INPUT                                  ;���� �������
    mov ds:[TIME], al
    lea dx,MSG_HELLO
    mov ah,9h
    int 21h
    mov ah,7h
    int 21h                                     ;�������� �������������
PREPARE_RTC:                                    ;���������� ����� RTC 
    mov al,0Bh
    out 70h,al                                  ;������� ������� ��������� 0Bh
    in  al,71h                                  ;�������� �������� �������� 0Bh
    or  al,00000100b                            ;���������� ��� DM � 1 - ������ ������������� ����� � �������� ����
    out 71h,al                                  ;�������� ���������� ��������
ENABLE_A20:                                     ;������� ����� A20
    in  al,92h                                                                              
    or  al,2                                    ;���������� ��� 1 � 1                                                   
    out 92h,al                                                                                                                     
    ;��� ��� ��� ������ �����������                                                                                                      0 LINE
    ;mov    al, 0D1h
    ;out    64h, al
    ;mov    al, 0DFh
    ;out    60h, al
SAVE_MASK:                                      ;��������� ����� ����������     
    in      al,21h
    mov     INT_MASK_M,al                  
    in      al,0A1h
    mov     INT_MASK_S,al                 
DISABLE_INTERRUPTS:                             ;������ ����������� � ������������� ����������        
    cli                                         ;������ ���������� ����������
    in  al,70h	
	or	al,10000000b                            ;���������� 7 ��� � 1 ��� ������� ������������� ����������
	out	70h,al
	nop	
LOAD_GDT:                                       ;��������� ���������� ������� ������������            
    mov ax,DATA
    mov dl,ah
    xor dh,dh
    shl ax,4
    shr dx,4
    mov si,ax
    mov di,dx
WRITE_GDT:                                      ;��������� ���������� GDT
    lea bx,GDT_GDT
    mov ax,si
    mov dx,di
    add ax,offset GDT
    adc dx,0
    mov [bx][S_DESC.BASE_L],ax
    mov [bx][S_DESC.BASE_M],dl
    mov [bx][S_DESC.BASE_H],dh
WRITE_CODE_RM:                                  ;��������� ���������� �������� ���� ��������� ������
    lea bx,GDT_CODE_RM
    mov ax,cs
    xor dh,dh
    mov dl,ah
    shl ax,4
    shr dx,4
    mov [bx][S_DESC.BASE_L],ax
    mov [bx][S_DESC.BASE_M],dl
    mov [bx][S_DESC.BASE_H],dh
WRITE_DATA:                                     ;�������� ���������� �������� ������
    lea bx,GDT_DATA
    mov ax,si
    mov dx,di
    mov [bx][S_DESC.BASE_L],ax
    mov [bx][S_DESC.BASE_M],dl
    mov [bx][S_DESC.BASE_H],dh
WRITE_STACK:                                    ;�������� ���������� �������� �����
    lea bx, GDT_STACK
    mov ax,ss
    xor dh,dh
    mov dl,ah
    shl ax,4
    shr dx,4
    mov [bx][S_DESC.BASE_L],ax
    mov [bx][S_DESC.BASE_M],dl
    mov [bx][S_DESC.BASE_H],dh
WRITE_CODE_PM:                                  ;�������� ���������� ���� ����������� ������
    lea bx,GDT_CODE_PM
    mov ax,CODE_PM
    xor dh,dh
    mov dl,ah
    shl ax,4
    shr dx,4
    mov [bx][S_DESC.BASE_L],ax
    mov [bx][S_DESC.BASE_M],dl
    mov [bx][S_DESC.BASE_H],dh        
    or  [bx][S_DESC.ATTRIBS],40h
WRITE_IDT:                                      ;�������� ���������� IDT
    lea bx,GDT_IDT
    mov ax,si
    mov dx,di
    add ax,OFFSET IDT
    adc dx,0
    mov [bx][S_DESC.BASE_L],ax
    mov [bx][S_DESC.BASE_M],dl
    mov [bx][S_DESC.BASE_H],dh        
    mov IDTR.IDT_L,ax
    mov IDTR.IDT_H,dx
FILL_IDT:                                       ;��������� ������� ������������ ������ ����������
    irpc    N, 0123456789ABCDEF                 ;��������� ����� 00-0F ������������
        lea eax, EXC_0&N
        mov IDT_0&N.OFFS_L,ax
        shr eax, 16
        mov IDT_0&N.OFFS_H,ax
    endm
    irpc    N, 0123456789ABCDEF                 ;��������� ����� 10-1F ������������
        lea eax, EXC_1&N
        mov IDT_1&N.OFFS_L,ax
        shr eax, 16
        mov IDT_1&N.OFFS_H,ax
    endm
    lea eax, TIMER_HANDLER                      ;��������� ���������� ���������� ������� �� 20 ����
    mov IDT_TIMER.OFFS_L,ax
    shr eax, 16
    mov IDT_TIMER.OFFS_H,ax
    lea eax, KEYBOARD_HANDLER                   ;��������� ���������� ���������� ���������� �� 21 ����
    mov IDT_KEYBOARD.OFFS_L,ax
    shr eax, 16
    mov IDT_KEYBOARD.OFFS_H,ax
    irpc    N, 234567                           ;��������� ������� 22-27 ����������
        lea eax,DUMMY_IRQ_MASTER
        mov IDT_2&N.OFFS_L, AX
        shr eax,16
        mov IDT_2&N.OFFS_H, AX
    endm
    irpc    N, 89ABCDEF                         ;��������� ������� 28-2F ����������
        lea eax,DUMMY_IRQ_SLAVE
        mov IDT_2&N.OFFS_L,ax
        shr eax,16
        mov IDT_2&N.OFFS_H,ax
    endm
    lgdt fword ptr GDT_GDT                      ;��������� ������� GDTR
    lidt fword ptr IDTR                         ;��������� ������� IDTR
    mov eax,cr0                                 ;�������� ����������� ������� cr0
    or  al,00000001b                            ;���������� ��� PE � 1
    mov cr0,eax                                 ;�������� ���������� cr0 � ��� ����� �������� ���������� �����
OVERLOAD_CS:                                    ;������������� ������� ���� �� ��� ����������
    db  0EAH
    dw  $+4
    dw  CODE_RM_DESC        
OVERLOAD_SEGMENT_REGISTERS:                     ;�������������������� ��������� ���������� �������� �� �����������
    mov ax,DATA_DESC
    mov ds,ax                         
    mov es,ax                         
    mov ax,STACK_DESC
    mov ss,ax                         
    xor ax,ax
    mov fs,ax                                   ;�������� ������� fs
    mov gs,ax                                   ;�������� ������� gs
    lldt ax                                     ;�������� ������� LDTR - �� ������������ ������� ��������� ������������
PREPARE_TO_RETURN:
    push cs                                     ;������� ����
    push offset BACK_TO_RM                      ;�������� ����� ��������
    lea  edi,ENTER_PM                           ;�������� ����� ����� � ���������� �����
    mov  eax,CODE_PM_DESC                       ;�������� ���������� ���� ����������� ������
    push eax                                    ;������� �� � ����
    push edi                                    
REINITIALIAZE_CONTROLLER_FOR_PM:                ;�������������������� ���������� ���������� �� ������� 20h, 28h
    mov al,00010001b                            ;ICW1 - ����������������� ����������� ����������
    out 20h,al                                  ;������������������ ������� ����������
    out 0A0h,al                                 ;������������������ ������� ����������
    mov al,20h                                  ;ICW2 - ����� �������� ������� ����������
    out 21h,al                                  ;�������� �����������
    mov al,28h                                  ;ICW2 - ����� �������� ������� ����������
    out 0A1h,al                                 ;�������� �����������
    mov al,04h                                  ;ICW3 - ������� ���������� ��������� � 3 �����
    out 21h,al       
    mov al,02h                                  ;ICW3 - ������� ���������� ��������� � 3 �����
    out 0A1h,al      
    mov al,11h                                  ;ICW4 - ����� ����������� ������ ����������� ��� �������� �����������
    out 21h,al        
    mov al,01h                                  ;ICW4 - ����� ������� ������ ����������� ��� �������� �����������
    out 0A1h,al       
    mov al, 0                                   ;�������������� ����������
    out 21h,al                                  ;�������� �����������
    out 0A1h,al                                 ;�������� �����������
ENABLE_INTERRUPTS_0:                            ;��������� ����������� � ������������� ����������
    in  al,70h	
	and	al,01111111b                            ;���������� 7 ��� � 0 ��� ������� ������������� ����������
	out	70h,al
	nop
    sti                                         ;��������� ����������� ����������
GO_TO_CODE_PM:                                  ;������� � �������� ���� ����������� ������
    db 66h                                      
    retf
BACK_TO_RM:                                     ;����� �������� � �������� �����
    cli                                         ;������ ����������� ����������
    in  al,70h	                                ;� �� ����������� ����������
	or	AL,10000000b                            ;���������� 7 ��� � 1 ��� ������� ������������� ����������
	out	70h,AL
	nop
REINITIALISE_CONTROLLER:                        ;���������������� ����������� ����������               
    mov al,00010001b                            ;ICW1 - ����������������� ����������� ����������
    out 20h,al                                  ;������������������ ������� ����������
    out 0A0h,al                                 ;������������������ ������� ����������
    mov al,8h                                   ;ICW2 - ����� �������� ������� ����������
    out 21h,al                                  ;�������� �����������
    mov al,70h                                  ;ICW2 - ����� �������� ������� ����������
    out 0A1h,al                                 ;�������� �����������
    mov al,04h                                  ;ICW3 - ������� ���������� ��������� � 3 �����
    out 21h,al       
    mov al,02h                                  ;ICW3 - ������� ���������� ��������� � 3 �����
    out 0A1h,al      
    mov al,11h                                  ;ICW4 - ����� ����������� ������ ����������� ��� �������� �����������
    out 21h,al        
    mov al,01h                                  ;ICW4 - ����� ������� ������ ����������� ��� �������� �����������
    out 0A1h,al
PREPARE_SEGMENTS:                               ;���������� ���������� ��������� ��� �������� � �������� �����          
    mov GDT_CODE_RM.LIMIT,0FFFFh                ;��������� ������ �������� ���� � 64KB
    mov GDT_DATA.LIMIT,0FFFFh                   ;��������� ������ �������� ������ � 64KB
    mov GDT_STACK.LIMIT,0FFFFh                  ;��������� ������ �������� ����� � 64KB
    db  0EAH                                    ;������������� ������� cs
    dw  $+4
    dw  CODE_RM_DESC                            ;�� ������� ���� ��������� ������
    mov ax,DATA_DESC                            ;�������� ���������� �������� ������������ �������� ������
    mov ds,ax                                   
    mov es,ax                                   
    mov fs,ax                                   
    mov gs,ax                                   
    mov ax,STACK_DESC
    mov ss,ax                                   ;�������� ������� ����� ������������ �����
ENABLE_REAL_MODE:                               ;������� �������� �����
    mov eax,cr0
    and al,11111110b                            ;������� 0 ��� �������� cr0
    mov cr0,eax                        
    db  0EAH
    dw  $+4
    dw  CODE_RM                                 ;������������ ������� ����
    mov ax,STACK_A
    mov ss,ax                      
    mov ax,DATA
    mov ds,ax                      
    mov es,ax
    xor ax,ax
    mov fs,ax
    mov gs,ax
    mov IDTR.LIMIT, 3FFH                
    mov dword ptr  IDTR+2, 0            
    lidt fword ptr IDTR                 
REPEAIR_MASK:                                   ;������������ ����� ����������
    mov al,INT_MASK_M
    out 21h,al                                  ;�������� �����������
    mov al,INT_MASK_S
    out 0A1h,al                                 ;�������� �����������
ENABLE_INTERRUPTS:                              ;��������� ����������� � ������������� ����������
    in  al,70h	
	and	al,01111111b                            ;���������� 7 ��� � 0 ��� ���������� ������������� ����������
	out	70h,al
    nop
    sti                                         ;��������� ����������� ����������
DISABLE_A20:                                    ;������� ������� A20
    in  al,92h
    and al,11111101b                            ;�������� 1 ��� - ��������� ����� A20
    out 92h, al
EXIT:                                           ;����� �� ���������
    mov ax,3h
    int 10H                                     ;�������� �����-�����    
    lea dx,MSG_EXIT
    mov ah,9h
    int 21h                                     ;������� ���������
    mov ax,4C00h
    int 21H                                     ;����� � dos
INPUT proc near                                 ;��������� ����� �����-���������� � ���������� ������ 
    mov ah,0ah
    xor di,di
    mov dx,offset ds:[INPUT_TIME]
    int 21h
    mov dl,0ah
    mov ah,02
    int 21h 
    
    mov si,offset INPUT_TIME+2 
    cmp byte ptr [si],"-" 
    jnz ii1
    mov di,1 
    inc si   
II1:
    xor ax,ax
    mov bx,10  
II2:
    mov cl,[si]
    cmp cl,0dh 
    jz ii3
    cmp cl,'0' 
    jl er
    cmp cl,'9' 
    ja er
 
    sub cl,'0' 
    mul bx     
    add ax,cx  
    inc si     
    jmp ii2    
ER:   
    mov dx, offset MSG_ERROR
    mov ah,09
    int 21h
    int 20h
II3:
    ret
INPUT endp
SIZE_CODE_RM    = ($ - CODE_RM_BEGIN)           ;����� �������� ����
CODE_RM ends
;������� ���� ��������� ������
CODE_PM  segment para use32
CODE_PM_BEGIN   = $
    assume cs:CODE_PM,ds:DATA,es:DATA           ;�������� ��������� ��� ����������
ENTER_PM:                                       ;����� ����� � ���������� �����
    call CLRSCR                                 ;��������� ������� ������
    xor  edi,edi                                ;� edi �������� �� ������
    lea  esi,MSG_HELLO_PM                       ;� esi ����� ������
    call BUFFER_OUTPUT                          ;������� ������-����������� � ���������� ������
    add  edi,160                                ;��������� ������ �� ��������� ������
    lea  esi,MSG_KEYBOARD
    call BUFFER_OUTPUT                          ;������� ���� ��� ������ ����-���� ����������
    mov edi,320
    lea esi,MSG_TIME
    call BUFFER_OUTPUT                          ;������� ���� ��� ������ �����
    mov edi,480
    lea esi,MSG_COUNT
    call BUFFER_OUTPUT
    mov DS:[COUNT],0
WAITING_ESC:                                    ;�������� ������� ������ ������ �� ����������� ������
    jmp  WAITING_ESC                            ;���� ��� ����� �� ESC
EXIT_PM:                                        ;����� ������ �� 32-������� �������� ����    
    db 66H
    retf                                        ;������� � 16-������ ������� ����
EXIT_FROM_INTERRUPT:                            ;����� ������ ��� ������ �������� �� ����������� ����������
    popad
    pop es
    pop ds
    pop eax                                     ;����� �� ����� ������ EIP
    pop eax                                     ;CS  
    pop eax                                     ;� EFLAGS
    sti                                         ;�����������, ��� ����� ��������� ���������� ���������� ���������
    db 66H
    retf                                        ;������� � 16-������ ������� ����    
WORD_TO_DEC proc near                           ;��������� �������� ����� � ������
    pushad    
    movzx eax,ax
    xor cx,cx              
    mov bx,10              
LOOP1:                                          ;���� �� ���������� �����             
    xor dx,dx              
    div bx                 
    add dl,'0'             
    push dx                
    inc cx                 
    test ax,ax             
    jnz LOOP1          
LOOP2:                                          ;���� �� ���������� ������                 
    pop dx                 
    mov [di],dl            
    inc di                 
    loop LOOP2         
    popad
    ret
WORD_TO_DEC endp
DIGIT_TO_HEX proc near                          ;��������� �������� ����� � ���������������� ���
    add al,'0'            
    cmp al,'9'            
    jle DTH_END           
    add al,7              
DTH_END:
    ret        
DIGIT_TO_HEX endp
BYTE_TO_HEX proc near                           ;��������� �������� ����� � ���������������� ���
    push ax
    mov ah,al             
    shr al,4              
    call DIGIT_TO_HEX     
    mov [di],al           
    inc di                
    mov al,ah             
    and al,0Fh            
    call DIGIT_TO_HEX     
    mov [di],al           
    inc di                
    pop ax
    ret    
BYTE_TO_HEX endp
M = 0                           
IRPC N, 0123456789ABCDEF
EXC_0&N label word                              ;����������� ����������
    cli 
    jmp EXC_HANDLER
endm
M = 010H
IRPC N, 0123456789ABCDEF                        ;����������� ����������
EXC_1&N label word                          
    cli
    jmp EXC_HANDLER
endm
EXC_HANDLER proc near                           ;��������� ������ ��������� ����������
    call CLRSCR                                 ;������� ������
    lea  esi, MSG_EXC
    mov  edi, 40*2
    call BUFFER_OUTPUT                          ;����� ��������������
    pop eax                                     ;����� �� ����� ������ EIP
    pop eax                                     ;CS  
    pop eax                                     ;� EFLAGS
    sti                                         ;�����������, ��� ����� ��������� ���������� ���������� ���������
    db 66H
    retf                                        ;������� � 16-������ ������� ����    
EXC_HANDLER     ENDP
DUMMY_IRQ_MASTER proc near                      ;�������� ��� ���������� ���������� �������� �����������
    push eax
    mov  al,20h
    out  20h,al
    pop  eax
    iretd
DUMMY_IRQ_MASTER endp
DUMMY_IRQ_SLAVE  proc near                      ;�������� ��� ���������� ���������� �������� �����������
    push eax
    mov  al,20h
    out  20h,al
    out  0A0h,al
    pop  eax
    iretd
DUMMY_IRQ_SLAVE  endp
TIMER_HANDLER proc near                         ;���������� ���������� ���������� �������
    push ds
    push es
    pushad                                      ;������� � ���� ����������� �������� ������ ���������� 
    mov  ax,DATA_DESC                           ;�������������������� ���������� ��������
    mov  ds,ax
    inc  ds:[COUNT]                             ;��������� �������� ��������
    lea  edi,ds:[BUFFER_COUNT]
    mov  ax,ds:[COUNT]
    call WORD_TO_DEC                            ;������������� ������� � ������
    mov  edi,538
    lea  esi,BUFFER_COUNT
    call BUFFER_OUTPUT                          ;������� �������� ��������
SHOW_TIMER:
    mov  al,0h                                  ;������� ������� ������ cmos
    out  70h,al
    in   al,71h                                 ;��������� �������� ������
    cmp  al,ds:[SECOND]                         ;���� ������� �� �� �����
    je   SKIP_SECOND                            ;�� ���������� �����
    mov  ds:[SECOND],al                         ;����� �������� �������� ����� �������
    mov  al,ds:[TIME]                           ;�������� �������� ����������� �����
    cmp  ds:[TIME],0                            ;���� ����� ������� � �����
    je   DISABLE_PM                             ;�� �� ����� �� ����������� ������
    xor  ah,ah
    lea  edi,ds:[BUFFER_TIME]                   
    call WORD_TO_DEC                            ;������������� ��� � ������
    mov  edi,422
    lea  esi,BUFFER_TIME                        
    call BUFFER_OUTPUT                          ;������� �������� ����������� �����
    dec  ds:[TIME]                              ;��������� �������� ����������� �������
    lea  esi,BUFFER_TIME
    call BUFFER_CLEAR                           ;������� ������
    jmp  SKIP_SECOND                            ;�� ����� �� ��������� �����
DISABLE_PM:                                     ;����� �� ����������� ������
    mov  al,20h
    out  20h,al
    db 0eah                                     ;������� jmp
    dd OFFSET EXIT_FROM_INTERRUPT               ;�� �����
    dw CODE_PM_DESC                             ;� ��������
SKIP_SECOND:                                    ;������� �� ��, �� ���� ����������� ������� ��������    
    mov  al,20h
    out  20h,al                                 ;�������� ������� ����������� ����������
    popad
    pop es
    pop ds
    iretd
TIMER_HANDLER endp
KEYBOARD_HANDLER proc near                      ;���������� ���������� ����������
    push ds
    push es
    pushad                                      ;��������� ����������� �������� ������ ����������
    in   al,60h                                 ;������� ���� ��� ��������� ������� �������                                ;
    cmp  al,1                                   ;���� ��� ����� 'ESC'
    je   KEYBOARD_EXIT                          ;����� �� ����� �� ����������� ������   
    mov  ds:[KEY_SCAN_CODE],al                  ;�������� ��� � ������
    lea  edi,ds:[BUFFER_SCAN_CODE]
    mov  al,ds:[KEY_SCAN_CODE]
    xor  ah,ah
    call BYTE_TO_HEX                            ;������������� ����-��� � ������
    mov  edi,200
    lea  esi,BUFFER_SCAN_CODE                   
    call BUFFER_OUTPUT                          ;������� ������ �� ����-�����
    jmp  KEYBOARD_RETURN  
KEYBOARD_EXIT:
    mov  al,20h
    out  20h,al
    db 0eah
    dd OFFSET EXIT_FROM_INTERRUPT 
    dw CODE_PM_DESC  
KEYBOARD_RETURN:
    mov  al,20h
    out  20h,al                                 ;�������� ������� ����������� ����������
    popad                                       ;������������ �������� ���������
    pop es
    pop ds
    iretd                                       ;����� �� ����������
KEYBOARD_HANDLER endp
CLRSCR  proc near                               ;��������� ������� �������
    push es
    pushad
    mov  ax,TEXT_DESC                           ;��������� � ax ���������� ������
    mov  es,ax
    xor  edi,edi
    mov  ecx,80*25                              ;���������� �������� � ����
    mov  ax,700h
    rep  stosw
    popad
    pop  es
    ret
CLRSCR  endp
BUFFER_CLEAR  proc near                         ;��������� ������� ������
    mov al,' '
    mov [esi+0],al
    mov [esi+1],al
    mov [esi+2],al
    mov [esi+3],al
    mov [esi+4],al
    mov [esi+5],al
    mov [esi+6],al
    mov [esi+7],al
    ret
BUFFER_CLEAR  endp
BUFFER_OUTPUT proc near                         ;��������� ������ ���������� ������, ��������������� 0
    push es
    PUSHAD
    mov  ax,TEXT_DESC                           ;��������� � es �������� ������
    mov  es,ax
OUTPUT_LOOP:                                    ;���� �� ������ ������
    lodsb                                       
    or   al,al
    jz   OUTPUT_EXIT                            ;���� ����� �� 0, �� ����� ������
    stosb
    inc  edi
    jmp  OUTPUT_LOOP
OUTPUT_EXIT:                                    ;����� �� ��������� ������
    popad
    pop  es
    ret
BUFFER_OUTPUT ENDP
SIZE_CODE_PM     =       ($ - CODE_PM_BEGIN)
CODE_PM  ENDS
;������� ������ ���������/����������� ������
DATA    segment para use16                      ;������� ������ ���������/����������� ������
DATA_BEGIN      = $
    ;GDT - ���������� ������� ������������
    GDT_BEGIN   = $
    GDT label   word                            ;����� ������ GDT (GDT: �� ��������)
    GDT_0       S_DESC <0,0,0,0,0,0>                              
    GDT_GDT     S_DESC <GDT_SIZE-1,,,ACS_DATA,0,>                 
    GDT_CODE_RM S_DESC <SIZE_CODE_RM-1,,,ACS_CODE,0,>             
    GDT_DATA    S_DESC <SIZE_DATA-1,,,ACS_DATA+ACS_DPL_3,0,>      
    GDT_STACK   S_DESC <1000h-1,,,ACS_DATA,0,>                    
    GDT_TEXT    S_DESC <2000h-1,8000h,0Bh,ACS_DATA+ACS_DPL_3,0,0> 
    GDT_CODE_PM S_DESC <SIZE_CODE_PM-1,,,ACS_CODE+ACS_READ,0,>    
    GDT_IDT     S_DESC <SIZE_IDT-1,,,ACS_IDT,0,>                  
    GDT_SIZE    = ($ - GDT_BEGIN)               ;������ GDT
    ;���������� ���������
    CODE_RM_DESC = (GDT_CODE_RM - GDT_0)
    DATA_DESC    = (GDT_DATA - GDT_0)      
    STACK_DESC   = (GDT_STACK - GDT_0)
    TEXT_DESC    = (GDT_TEXT - GDT_0)  
    CODE_PM_DESC = (GDT_CODE_PM - GDT_0)
    IDT_DESC     = (GDT_IDT - GDT_0)
    ;IDT - ������� ������������ ����������
    IDTR    R_IDTR  <SIZE_IDT,0,0>              ;������ �������� ITDR   
    IDT label   word                            ;����� ������ IDT
    IDT_BEGIN   = $
    IRPC    N, 0123456789ABCDEF
        IDT_0&N I_DESC <0, CODE_PM_DESC,0,ACS_TRAP,0>            ; 00...0F
    ENDM
    IRPC    N, 0123456789ABCDEF
        IDT_1&N I_DESC <0, CODE_PM_DESC, 0, ACS_TRAP, 0>         ; 10...1F
    ENDM
    IDT_TIMER    I_DESC <0,CODE_PM_DESC,0,ACS_INT,0>             ;IRQ 0 - ���������� ���������� �������
    IDT_KEYBOARD I_DESC <0,CODE_PM_DESC,0,ACS_INT,0>             ;IRQ 1 - ���������� ����������
    IRPC    N, 23456789ABCDEF
        IDT_2&N         I_DESC <0, CODE_PM_DESC, 0, ACS_INT, 0>  ; 22...2F
    ENDM
    SIZE_IDT        =       ($ - IDT_BEGIN)
    MSG_HELLO           db "press any key to go to the protected mode",13,10,"$"
    MSG_HELLO_PM        db "wellcome in protected mode",0
    MSG_EXIT            db "wellcome back to real mode",13,10,"$"
    MSG_KEYBOARD        db "keyboard scan code:            | press 'ESC' to come back to the real mode",0
    MSG_TIME            db "                               | go back to RM in  XXXXXXX seconds",0
    MSG_COUNT           db "quantity of interrupt calls:",0
    MSG_EXC             db "exception: XX",0
    MSG_ENTER           db "enter time in protected mode: $"
    MSG_ERROR           db "incorrect error$"
    HEX_TAB             db "0123456789ABCDEF"   ;������� ������� ����������
    ESP32               dd  1 dup(?)            ;��������� �� ������� �����
    INT_MASK_M          db  1 dup(?)            ;�������� �������� ����� �������� �����������
    INT_MASK_S          db  1 dup(?)            ;�������� �������� ����� �������� �����������
    KEY_SCAN_CODE       db  1 dup(?)            ;���-��� ��������� ������� �������
    SECOND              db  1 dup(?)            ;������� �������� ������
    TIME                db  1 dup(10)           ;����� ���������� � ���������� ������
    COUNT               dw  1 dup(0)            ;���������� ������� ���������� (�������� �� 0 �� 65535)
    BUFFER_COUNT        db  8 dup(' ')          ;����� ��� ������ ���������� ������� ����������
                        db  1 dup(0)
    BUFFER_SCAN_CODE    db  8 dup(' ')          ;����� ��� ������ ����-���� ����������
                        db  1 dup(0)                
    BUFFER_TIME         db  8 dup(' ')          ;����� ��� ������ ����������� ����� � ���������� ������
                        db  1 dup(0)
    INPUT_TIME          db  6,7 dup(?)          ;����� ��� ����� �����        
SIZE_DATA   = ($ - DATA_BEGIN)                  ;������ �������� ������
DATA    ends
;������� ����� ���������/����������� ������
STACK_A segment para stack
    db  1000h dup(?)
STACK_A  ends
end START

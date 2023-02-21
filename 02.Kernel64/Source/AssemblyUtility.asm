[BITS 64]           ; 이하의 코드는 64비트 코드로 설정

SECTION .text

global kInPortByte, kOutPortByte, kInPortWord, kOutPortWord
global kLoadGDTR, kLoadTR, kLoadIDTR
global kEnableInterrupt, kDisableInterrupt, kReadRFLAGS
global kReadTSC
global kSwitchContext, kHlt, kTestAndSet
global kInitializeFPU, kSaveFPUContext, kLoadFPUContext, kSetTS, kClearTS

      ; text 섹션(세그먼트)을 정의


; 포트로부터 1바이트를 읽음
;   PARAM: 포트 번호
kInPortByte:
    push rdx        ; 함수에서 임시로 사용하는 레지스터를 스택에 저장
                    ; 함수의 마지막 부분에서 스택에 삽입된 값을 꺼내 복원

    mov rdx, rdi    ; RDX 레지스터에 파라미터 1(포트 번호)를 저장
    mov rax, 0      ; RAX 레지스터를 초기화
    in al, dx       ; DX 레지스터에 저장된 포트 어드레스에서 한 바이트를 읽어
                    ; AL 레지스터에 저장, AL 레지스터는 함수의 반환 값으로 사용됨

    pop rdx         ; 함수에서 사용이 끝난 레지스터를 복원
    ret             ; 함수를 호출한 다음 코드의 위치로 복귀
    
; 포트에 1바이트를 씀
;   PARAM: 포트 번호, 데이터
kOutPortByte:
    push rdx        ; 함수에서 임시로 사용하는 레지스터를 스택에 저장
    push rax        ; 함수의 마지막 부분에서 스택에 삽입된 값을 꺼내 복원
    
    mov rdx, rdi    ; RDX 레지스터에 파라미터 1(포트 번호)를 저장
    mov rax, rsi    ; RAX 레지스터에 파라미터 2(데이터)를 저장
    out dx, al      ; DX 레지스터에 저장된 포트 어드레스에 AL 레지스터에 저장된
                    ; 한 바이트를 
    pop rax         ; 함수에서 사용이 끝난 레지스터를 복원
    pop rdx
    ret             ; 함수를 호출한 다음 코드의 위치로 복귀

; GDTR 레지스터에 GDT 테이블을 설정
; PARAM: GDT 테이블의 정보를 저장하는 자료구조의 어드레스
kLoadGDTR:
	lgdt[rdi]

	ret

; TR 레지스터에 TSS 세그먼트 디스크립터 설정
; PARAM: TSS 세그먼트 디스크립터의 오프셋
kLoadTR:
	ltr di

	ret

; IDTR 레지스터에 IDT 테이블을 설정
; PARAM: IDT 테이블의 정보를 저장하는 자료구조의 어드레스
kLoadIDTR:
	lidt[rdi]
	
	ret

kEnableInterrupt:
	sti
	ret

kDisableInterrupt:
	cli
	ret

kReadRFLAGS:
	pushfq
	pop rax

	ret

kReadTSC:
	push rdx

	rdtsc

	shl rdx, 32
	or rax, rdx

	pop rdx
	ret

; 콘텍스트를 저장하고 셀렉터를 교체하는 매크로
%macro KSAVECONTEXT 0
	push rbp
	push rax
	push rbx
	push rcx
	push rdx
	push rdi
	push rsi
	push r8
	push r9
	push r10
	push r11
	push r12
	push r13
	push r14
	push r15

	mov ax, ds
	push rax
	mov ax, es
	push rax
	push fs
	push gs
%endmacro


%macro KLOADCONTEXT 0
	pop gs
	pop fs
	pop rax
	mov es, ax
	pop rax
	mov ds, ax

	pop r15
	pop r14
	pop r13
	pop r12
	pop r11
	pop r10
	pop r9
	pop r8
	pop rsi
	pop rdi
	pop rdx
	pop rcx	
	pop rbx	
	pop rax
	pop rbp
%endmacro

; Current Context에 현재 콘텍스트를 저장하고 Next Task에서 콘텍스트를 복구
; PARAM: Current Context, Next Context
kSwitchContext:
	push rbp
	mov rbp, rsp

	pushfq ; Current Context가 NULL이면 콘텍스트를 저장할 필요 없음
	cmp rdi, 0
	je .LoadContext
	popfq




;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
 ; 현재 태스크의 콘텍스트를 저장
 ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	push rax

	mov ax, ss
	mov qword[rdi + (23 * 8) ], rax

	mov rax, rbp
	add rax, 16
	mov qword[ rdi+(22 * 8 ) ],rax

	pushfq
	pop rax
	mov qword[rdi + ( 21 * 8 ) ], rax

	mov ax, cs
	mov qword[ rdi + ( 20 * 8 ) ], rax

	mov rax, qword[rbp + 8 ]
	mov qword[ rdi + ( 19 * 8 ) ],rax
; 저장한 레지스터를 복구한 후 인터럽트가 발생했을 때처럼 나머지 콘텍스트를 모두 저장
	pop rax
	pop rbp

 ; 가장 끝부분에 SS, RSP, RFLAGS, CS, RIP 레지스터를 저장했으므로, 이전 영역에
 ; push 명령어로 콘텍스트를 저장하기 위해 스택을 변경
	add rdi, (19*8)
	mov rsp, rdi
	sub rdi, (19* 8 )

; 나머지 레지스터를 모두 Context 자료구조에 저장
	KSAVECONTEXT

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
 ; 다음 태스크의 콘텍스트 복원
 ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
.LoadContext:

	mov rsp, rsi	
	
	KLOADCONTEXT
	iretq


; 프로세서를 쉬게 함
; PARAM: 없음
kHlt:
	hlt
	hlt
	ret

; 테스트와 설정을 하나의 명령으로 처리
; Destination과 Compare를 비교하여 같다면, Desination에 Source 값을 삽입
; PARAM: 값을 저장할 어드레스(Destination, rdi), 비교할 값(Compare, rsi),
; 설정할 값(Source, rdx)
kTestAndSet:
	mov rax, rsi

	lock cmpxchg byte [rdi], dl
	je .SUCCESS

.NOTSAME:
	mov rax, 0x00
	ret
	
.SUCCESS:
	mov rax,0x01
	ret

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; FPU 관련 어셈블리어 함수
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; FPU를 초기화
; PAPAM: 없음
kInitializeFPU:
	finit
	ret

;FPU 관련 레지스터를 콘텍스트 버퍼에 저장
; PARAM: BUffer Address
kSaveFPUContext:
	fxsave	[rdi]
	ret

;FPU 관련 레지스터를 콘텍스트 버퍼에서 복원
;	PARAM: Buffer Address
kLoadFPUContext:
	fxrstor [rdi]
	ret

;CR0 컨트롤 레지스터의 TS 비트를 1로 설정
kSetTS:
	push rax

	mov rax, cr0
	or rax, 0x08
	mov cr0,rax

	pop rax
	ret

;CR0 컨트롤 레지스터의 TS 비트를 0으로 설정
kClearTS:
	clts
	ret

;포트로부터 2바이트를 읽음
kInPortWord:
	push rdx

	mov rdx, rdi
	mov rax,0
	in ax,dx

	pop rdx
	ret

kOutPortWord:
	push rdx
	push rax

	mov rdx, rdi
	mov rax, rsi
	out dx, ax

	pop rax
	pop rdx
	ret
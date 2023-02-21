[BITS 32]

global kReadCPUID, kSwitchAndExecute64bitKernel

SECTION .text

kReadCPUID:
	push ebp
	mov ebp, esp
	push eax
	push ebx
	push ecx
	push edx
	push esi

	;;;;;;;;;;;;;;;;;
	;EAX 레지스터의 값으로 CPUID 명령어 실행
	;;;;;;;;;;;;;;;;;
	mov eax, dword[ ebp + 8 ]
	cpuid


	;;;;;;;;;;;;;;;;;;;;;;;
	;반환된 값을 파라미터에 저장
	;;;;;;;;;;;;;;;;;;;;;;
	;*pdweax
	mov esi, dword[ebp + 12]
	mov dword [esi],eax


	;pdwebx
	mov esi, dword[ebp + 16]
	mov dword[esi],ebx

	;pdwecx
	mov esi, dword [ebp + 20]
	mov dword[esi],ecx

	;pdw edx
	mov esi, dword[ebp+24]
	mov dword[esi],edx

	pop esi
	pop edx
	pop ecx
	pop ebx
	pop eax
	pop ebp
	ret
	

	;IA-32e모드로 전환하고 64비트 커널을 수행
kSwitchAndExecute64bitKernel:
	;;;;;;;;;;;;;;;;;;;;;;;;
	;CR4 컨트롤 레지스터의 PAE 비트와 OSXMMEXCPT 비트, OSFXSR 비트를 1로 설정
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	mov eax, cr4
	or eax, 0x620
	mov cr4, eax

	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	;CR3 컨트롤 레지스터에 PML4 테이블의 어드레스와 캐시 활성화
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	mov eax, 0x100000
	mov cr3, eax

	;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	;CR3 컨트롤 레지스터에 PML4 테이블의 어드레스와 캐시 활성화
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	mov ecx, 0xC0000080
	rdmsr

	or eax, 0x0100
	
	wrmsr

	;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	; CR0 컨트롤 레지스터를 NW 비트(비트 29) = 0, CD 비트(비트 30) = 0, PG 비트(비트 31) = 1로
	;설정하여 캐시 기능과 페이징 기능을 활성화
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	mov eax, cr0
	or eax, 0xE000000E
	xor eax, 0x60000004
	mov cr0, eax


	jmp 0x08:0x200000


	;여기는 실행되지 않음
	jmp $

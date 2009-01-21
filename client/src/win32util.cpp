#ifdef _WINDOWS

//From http://www.suacommunity.com/forum/tm.aspx?m=7904

extern "C" void _chkstk();
 
 //_alloca_probe_16 : 16 byte aligned alloca
 extern "C" void _alloca_probe_16()
 {
 	
 }
 
 //alloca_8: 8 byte aligned alloca
 extern "C" void _alloca_probe_8()
 {
 	__asm 
 	{
 		push    ecx
 		lea     ecx, [esp] + 8          ; TOS before entering this function
 		sub     ecx, eax                ; New TOS
 		and     ecx, (8 - 1)            ; Distance from 8 bit align (align down)
 		add     eax, ecx                ; Increase allocation Size
 		sbb     ecx, ecx                ; ecx = 0xFFFFFFFF if size wrapped around
 		or      eax, ecx                ; cap allocation size on wraparound
 		pop     ecx                     ; Restore ecx
 		jmp     _chkstk
 	}
 }

#endif
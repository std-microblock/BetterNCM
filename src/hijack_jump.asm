
;
; created by AheadLib
; github:https://github.com/strivexjun/AheadLib-x86-x64
;
; 把 .asm 文件添加到工程一次
; 右键单击文件-属性-常规-
; 项类型:自定义生成工具
; 从生成中排除:否

; 然后复制下面命令填入
; 命令行: ml64 /Fo $(IntDir)%(fileName).obj /c /Cp %(fileName).asm
; 输出: $(IntDir)%(fileName).obj;%(Outputs)
; 链接对象: 是
;


.DATA
EXTERN pfnAheadLib_vSetDdrawflag:dq;
EXTERN pfnAheadLib_AlphaBlend:dq;
EXTERN pfnAheadLib_DllInitialize:dq;
EXTERN pfnAheadLib_GradientFill:dq;
EXTERN pfnAheadLib_TransparentBlt:dq;

.CODE
AheadLib_vSetDdrawflag PROC
	jmp pfnAheadLib_vSetDdrawflag
AheadLib_vSetDdrawflag ENDP

AheadLib_AlphaBlend PROC
	jmp pfnAheadLib_AlphaBlend
AheadLib_AlphaBlend ENDP

AheadLib_DllInitialize PROC
	jmp pfnAheadLib_DllInitialize
AheadLib_DllInitialize ENDP

AheadLib_GradientFill PROC
	jmp pfnAheadLib_GradientFill
AheadLib_GradientFill ENDP

AheadLib_TransparentBlt PROC
	jmp pfnAheadLib_TransparentBlt
AheadLib_TransparentBlt ENDP


END

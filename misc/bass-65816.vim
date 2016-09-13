" Vim syntax file
" Language: 65816 assembly (bass assembler)
" Maintainer: Marcus Rowe
" Latest Revision: 13 September 2016

if exists("b:current_syntax")
  finish
endif

syn case match

syn match bassName /[:_A-Za-z\.][:_A-Za-z0-9\.]*/

syn match bassLabel /^\s*[:_A-Za-z\.][:_A-Za-z0-9\.]*:/
syn match bassLabel /^\s*-/
syn match bassLabel /^\s*+/

" Opcodes
" Annoyingly syn-keyword cannot contain a '.' character
syn keyword bassOpcode nextgroup=bassOpcodeSize adc and asl bcc bcs beq bit bmi bne bpl
syn keyword bassOpcode nextgroup=bassOpcodeSize bra brk brl bvc bvs clc cld cli clv cmp
syn keyword bassOpcode nextgroup=bassOpcodeSize cop cpx cpy dec dex dey eor inc inx iny
syn keyword bassOpcode nextgroup=bassOpcodeSize jml jmp jsl jsr lda ldx ldy lsr mvn mvp
syn keyword bassOpcode nextgroup=bassOpcodeSize nop ora pea pei per pha phb phd phk php
syn keyword bassOpcode nextgroup=bassOpcodeSize phx phy pla plb pld plp plx ply rep rol
syn keyword bassOpcode nextgroup=bassOpcodeSize ror rti rtl rts sbc sec sed sei sep sta
syn keyword bassOpcode nextgroup=bassOpcodeSize stp stx sty stz tad tas tax tay tcd tda
syn keyword bassOpcode nextgroup=bassOpcodeSize tdc trb tsa tsb tsx txa txs txy tya tyx
syn keyword bassOpcode nextgroup=bassOpcodeSize wai wdm xba xce

syn match bassOpcodeSize "\.b" contained
syn match bassOpcodeSize "\.w" contained
syn match bassOpcodeSize "\.l" contained


" Constants
syn match bassNumber /\d\+/
syn match bassNumber /0b[01']\+/
syn match bassNumber /%[01']\+/
syn match bassNumber /0o[0-7']\+/
syn match bassNumber /\$[0-9a-fA-F']\+/
syn match bassNumber /0x[0-9a-fA-F']\+/

syn match bassCharacter /'.'/
syn match bassCharacter /'\\.'/
syn region bassString start=/"/ end=/"/ skip=/\\"/ contains=@Spell


" Macros
syn keyword bassMacro macro nextgroup=bassMacroName,bassMacroScope skipwhite
syn keyword bassMacroScope scope nextgroup=bassMacroName skipwhite
syn match   bassMacroName /[a-zA-Z0-9._]\+/ contained nextgroup=bassMacroParam,bassMacroParamError
syn region  bassMacroParam start=/(/ end=/)/ contained oneline
syn match   bassMacroArgument /[a-zA-Z0-9._]\+/ contained containedin=bassMacroParam

syn keyword  bassMacroKeyword define contained containedin=bassMacroParam
syn keyword  bassMacroKeyword evaluate contained containedin=bassMacroParam
syn keyword  bassMacroKeyword variable contained containedin=bassMacroParam

" Show error if space between macro name and parameter list
syn match   bassMacroParamError /[^(]\+/ contained nextgroup=bassMacroParam


" Scopes
syn keyword bassScope scope nextgroup=bassScopeLabel,bassScopeName skipwhite
syn match   bassScopeName /[a-zA-Z0-9._]\+/ contained display
syn match   bassScopeLabel /[a-zA-Z0-9._]\+:/ contained display


" Conditions
syn keyword bassConditional if else


" bass Keywords
syn keyword bassKeyword arch global define variable evaluate constant print include
syn keyword bassKeyword output endian origin base pushvar pullvar insert fill map
syn keyword bassKeyword db dw dl dd dq float32 float64
syn keyword bassKeyword notice warning error


" bass Functions
syn match bassFunction /origin()/
syn match bassFunction /base()/
syn match bassFunction /pc()/
syn match bassFunction /putchar(.*)/

" Comments
syn region bassComment start="//" end="$" keepend contains=@Spell


hi def link bassError Error
hi def link bassLabel Identifier
hi def link bassOpcode Statement
hi def link bassOpcodeSize Statement
hi def link bassNumber Constant
hi def link bassCharacter Character
hi def link bassString String
hi def link bassMacro Structure
hi def link bassMacroScope Structure
hi def link bassMacroName Function
hi def link bassMacroArgument Special
hi def link bassMacroKeyword Keyword
hi def link bassMacroParamError Error
hi def link bassScope Structure
hi def link bassScopeLabel Identifier
hi def link bassScopeName Special
hi def link bassConditional Structure
hi def link bassFunction Special
hi def link bassKeyword PreProc
hi def link bassComment Comment


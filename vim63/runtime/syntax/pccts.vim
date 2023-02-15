" Vim syntax file
" Language:	PCCTS
" Maintainer:	Scott Bigham <dsb@cs.duke.edu>
" Last Change:	10 Aug 1999

" For version 5.x: Clear all syntax items
" For version 6.x: Quit when a syntax file was already loaded
if version < 600
  syntax clear
elseif exists("b:current_syntax")
  finish
endif

" Read the C++ syntax to start with
if version < 600
  syn include @cppTopLevel <sfile>:p:h/cpp.vim
else
  syn include @cppTopLevel syntax/cpp.vim
endif

syn region pcctsAction matchgroup=pcctsDelim start="<<" end=">>?\=" contains=@cppTopLevel,pcctsRuleRef

syn region pcctsArgBlock matchgroup=pcctsDelim start="\(>\s*\)\=\[" end="\]" contains=@cppTopLevel,pcctsRuleRef

syn region pcctsString start=+"+ skip=+\\\\\|\\"+ end=+"+ contains=pcctsSpecialChar
syn match  pcctsSpecialChar "\\\\\|\\\"" contained

syn region pcctsComment start="/\*" end="\*/" contains=cTodo
syn match  pcctsComment "//.*$" contains=cTodo

syn region pcctsDirective start="^\s*#header\s\+<<" end=">>" contains=pcctsAction keepend
syn match  pcctsDirective "^\s*#parser\>.*$" contains=pcctsString,pcctsComment
syn match  pcctsDirective "^\s*#tokdefs\>.*$" contains=pcctsString,pcctsComment
syn match  pcctsDirective "^\s*#token\>.*$" contains=pcctsString,pcctsAction,pcctsTokenName,pcctsComment
syn region pcctsDirective start="^\s*#tokclass\s\+[A-Z]\i*\s\+{" end="}" contains=pcctsString,pcctsTokenName
syn match  pcctsDirective "^\s*#lexclass\>.*$" contains=pcctsTokenName
syn region pcctsDirective start="^\s*#errclass\s\+[^{]\+\s\+{" end="}" contains=pcctsString,pcctsTokenName
syn match pcctsDirective "^\s*#pred\>.*$" contains=pcctsTokenName,pcctsAction

syn cluster pcctsInRule contains=pcctsString,pcctsRuleName,pcctsTokenName,pcctsAction,pcctsArgBlock,pcctsSubRule,pcctsLabel,pcctsComment

syn region pcctsRule start="\<[a-z][A-Za-z0-9_]*\>\(\s*\[[^]]*\]\)\=\(\s*>\s*\[[^]]*\]\)\=\s*:" end=";" contains=@pcctsInRule

syn region pcctsSubRule matchgroup=pcctsDelim start="(" end=")\(+\|\*\|?\(\s*=>\)\=\)\=" contains=@pcctsInRule contained
syn region pcctsSubRule matchgroup=pcctsDelim start="{" end="}" contains=@pcctsInRule contained

syn match pcctsRuleName  "\<[a-z]\i*\>" contained
syn match pcctsTokenName "\<[A-Z]\i*\>" contained

syn match pcctsLabel "\<\I\i*:\I\i*" contained contains=pcctsLabelHack,pcctsRuleName,pcctsTokenName
syn match pcctsLabel "\<\I\i*:\"\([^\\]\|\\.\)*\"" contained contains=pcctsLabelHack,pcctsString
syn match pcctsLabelHack "\<\I\i*:" contained

syn match pcctsRuleRef "\$\I\i*\>" contained
syn match pcctsRuleRef "\$\d\+\(\.\d\+\)\>" contained

syn keyword pcctsClass     class   nextgroup=pcctsClassName skipwhite
syn match   pcctsClassName "\<\I\i*\>" contained nextgroup=pcctsClassBlock skipwhite skipnl
syn region pcctsClassBlock start="{" end="}" contained contains=pcctsRule,pcctsComment,pcctsDirective,pcctsAction,pcctsException,pcctsExceptionHandler

syn keyword pcctsException exception nextgroup=pcctsExceptionRuleRef skipwhite
syn match pcctsExceptionRuleRef "\[\I\i*\]" contained contains=pcctsExceptionID
syn match pcctsExceptionID "\I\i*" contained
syn keyword pcctsExceptionHandler	catch default
syn keyword pcctsExceptionHandler	NoViableAlt NoSemViableAlt
syn keyword pcctsExceptionHandler	MismatchedToken

syn sync clear
syn sync match pcctsSyncAction grouphere pcctsAction "<<"
syn sync match pcctsSyncAction "<<\([^>]\|>[^>]\)*>>"
syn sync match pcctsSyncRule grouphere pcctsRule "\<[a-z][A-Za-z0-9_]*\>\s*\[[^]]*\]\s*:"
syn sync match pcctsSyncRule grouphere pcctsRule "\<[a-z][A-Za-z0-9_]*\>\(\s*\[[^]]*\]\)\=\s*>\s*\[[^]]*\]\s*:"

" Define the default highlighting.
" For version 5.7 and earlier: only when not done already
" For version 5.8 and later: only when an item doesn't have highlighting yet
if version >= 508 || !exists("did_pccts_syntax_inits")
  if version < 508
    let did_pccts_syntax_inits = 1
    command -nargs=+ HiLink hi link <args>
  else
    command -nargs=+ HiLink hi def link <args>
  endif

  HiLink pcctsDelim		Special
  HiLink pcctsTokenName		Identifier
  HiLink pcctsRuleName		Statement
  HiLink pcctsLabelHack		Label
  HiLink pcctsDirective		PreProc
  HiLink pcctsString		String
  HiLink pcctsComment		Comment
  HiLink pcctsClass		Statement
  HiLink pcctsClassName		Identifier
  HiLink pcctsException		Statement
  HiLink pcctsExceptionHandler	Keyword
  HiLink pcctsExceptionRuleRef	pcctsDelim
  HiLink pcctsExceptionID	Identifier
  HiLink pcctsRuleRef		Identifier
  HiLink pcctsSpecialChar	SpecialChar

  delcommand HiLink
endif

let b:current_syntax = "pccts"

" vim: ts=8

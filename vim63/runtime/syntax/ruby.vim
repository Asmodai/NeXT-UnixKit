" Vim syntax file
" Language:		Ruby
" Maintainer:		Doug Kearns
" Previous Maintainer:	Mirko Nasato
" Last Change:		2003 May 31
" URL:			http://mugca.its.monash.edu.au/~djkea2/vim/syntax/ruby.vim

" $Id: ruby.vim,v 1.43 2003/04/01 08:48:19 doug Exp doug $

" Thanks to perl.vim authors, and to Reimer Behrends. :-) (MN)

" For version 5.x: Clear all syntax items
" For version 6.x: Quit when a syntax file was already loaded
if version < 600
  syntax clear
elseif exists("b:current_syntax")
  finish
endif

" Expression Substitution and Backslash Notation
syn match rubyExprSubst "\\\\\|\(\(\\M-\\C-\|\\c\|\\C-\|\\M-\)\w\)\|\(\\\o\{3}\|\\x\x\{2}\|\\[abefnrstv]\)" contained
syn match rubyExprSubst "#{[^}]*}" contained
syn match rubyExprSubst "#[$@]\w\+" contained

" Numbers and ASCII Codes
syn match rubyNumber "\w\@<!\(?\(\\M-\\C-\|\\C-\\M-\|\\M-\\c\|\\c\\M-\|\\c\|\\C-\|\\M-\)\=\(\\\o\{3}\|\\x\x\{2}\|\\\=\S\)\)"
syn match rubyNumber "\<\(0x\x\+\|0b[01]\+\|0\o\+\|0\.\d\+\|0\|[1-9][\.0-9_]*\)\>"

" Identifiers - constant, class and instance, global, symbol, iterator, predefined
syn match rubyLocalVariableOrMethod "[_[:lower:]][_[:alnum:]]*[?!=]\=" transparent contains=NONE

if !exists("ruby_no_identifiers")
  syn match rubyConstant		"\(::\)\=\zs\u\w*"
  syn match rubyClassVariable		"@@\h\w*"
  syn match rubyInstanceVariable	"@\h\w*"
  syn match rubyGlobalVariable		"$\(\h\w*\|-.\)"
  syn match rubySymbol			":\@<!:\(\$\|@@\=\)\=\h\w*[?!=]\="
  syn match rubyIterator		"|[ ,a-zA-Z0-9_*]\+|"

  syn match rubyPredefinedVariable "$[!"$&'*+,./0:;<=>?@\\_`~1-9]"
  syn match rubyPredefinedVariable "$-[0FIKadilpvw]"
  syn match rubyPredefinedVariable "$\(defout\|stderr\|stdin\|stdout\)\>"
  syn match rubyPredefinedVariable "$\(DEBUG\|FILENAME\|KCODE\|LOAD_PATH\|SAFE\|VERBOSE\)\>"
  syn match rubyPredefinedConstant "__\(FILE\|LINE\)__\>"
  syn match rubyPredefinedConstant "\<\(::\)\=\zs\(MatchingData\|NotImplementError\|ARGF\|ARGV\|ENV\)\>"
  syn match rubyPredefinedConstant "\<\(::\)\=\zs\(DATA\|FALSE\|NIL\|RUBY_PLATFORM\|RUBY_RELEASE_DATE\)\>"
  syn match rubyPredefinedConstant "\<\(::\)\=\zs\(RUBY_VERSION\|STDERR\|STDIN\|STDOUT\|TOPLEVEL_BINDING\|TRUE\)\>"
  "Obsolete Global Constants
  "syn match rubyPredefinedConstant "\<\(::\)\=\zs\(PLATFORM\|RELEASE\|VERSION\)\>"
endif

"
" BEGIN Autogenerated Stuff
"
" Generalized Regular Expression
syn region rubyString matchgroup=rubyStringDelimit start="%r!"  end="![iomx]*"  skip="\\\\\|\\!"  contains=rubyExprSubst fold
syn region rubyString matchgroup=rubyStringDelimit start="%r\"" end="\"[iomx]*" skip="\\\\\|\\\"" contains=rubyExprSubst fold
syn region rubyString matchgroup=rubyStringDelimit start="%r#"  end="#[iomx]*"  skip="\\\\\|\\#"  contains=rubyExprSubst fold
syn region rubyString matchgroup=rubyStringDelimit start="%r\$" end="\$[iomx]*" skip="\\\\\|\\\$" contains=rubyExprSubst fold
syn region rubyString matchgroup=rubyStringDelimit start="%r%"  end="%[iomx]*"  skip="\\\\\|\\%"  contains=rubyExprSubst fold
syn region rubyString matchgroup=rubyStringDelimit start="%r&"  end="&[iomx]*"  skip="\\\\\|\\&"  contains=rubyExprSubst fold
syn region rubyString matchgroup=rubyStringDelimit start="%r'"  end="'[iomx]*"  skip="\\\\\|\\'"  contains=rubyExprSubst fold
syn region rubyString matchgroup=rubyStringDelimit start="%r\*" end="\*[iomx]*" skip="\\\\\|\\\*" contains=rubyExprSubst fold
syn region rubyString matchgroup=rubyStringDelimit start="%r+"  end="+[iomx]*"  skip="\\\\\|\\+"  contains=rubyExprSubst fold
syn region rubyString matchgroup=rubyStringDelimit start="%r-"  end="-[iomx]*"  skip="\\\\\|\\-"  contains=rubyExprSubst fold
syn region rubyString matchgroup=rubyStringDelimit start="%r\." end="\.[iomx]*" skip="\\\\\|\\\." contains=rubyExprSubst fold
syn region rubyString matchgroup=rubyStringDelimit start="%r/"  end="/[iomx]*"  skip="\\\\\|\\/"  contains=rubyExprSubst fold
syn region rubyString matchgroup=rubyStringDelimit start="%r:"  end=":[iomx]*"  skip="\\\\\|\\:"  contains=rubyExprSubst fold
syn region rubyString matchgroup=rubyStringDelimit start="%r;"  end=";[iomx]*"  skip="\\\\\|\\;"  contains=rubyExprSubst fold
syn region rubyString matchgroup=rubyStringDelimit start="%r="  end="=[iomx]*"  skip="\\\\\|\\="  contains=rubyExprSubst fold
syn region rubyString matchgroup=rubyStringDelimit start="%r?"  end="?[iomx]*"  skip="\\\\\|\\?"  contains=rubyExprSubst fold
syn region rubyString matchgroup=rubyStringDelimit start="%r@"  end="@[iomx]*"  skip="\\\\\|\\@"  contains=rubyExprSubst fold
syn region rubyString matchgroup=rubyStringDelimit start="%r\\" end="\\[iomx]*"			  contains=rubyExprSubst fold
syn region rubyString matchgroup=rubyStringDelimit start="%r\^" end="\^[iomx]*" skip="\\\\\|\\\^" contains=rubyExprSubst fold
syn region rubyString matchgroup=rubyStringDelimit start="%r`"  end="`[iomx]*"  skip="\\\\\|\\`"  contains=rubyExprSubst fold
syn region rubyString matchgroup=rubyStringDelimit start="%r|"  end="|[iomx]*"  skip="\\\\\|\\|"  contains=rubyExprSubst fold
syn region rubyString matchgroup=rubyStringDelimit start="%r\~" end="\~[iomx]*" skip="\\\\\|\\\~" contains=rubyExprSubst fold
syn region rubyString matchgroup=rubyStringDelimit start="%r{"  end="}[iomx]*"  skip="\\\\\|\\}"  contains=rubyExprSubst fold
syn region rubyString matchgroup=rubyStringDelimit start="%r<"  end=">[iomx]*"  skip="\\\\\|\\>"  contains=rubyExprSubst fold
syn region rubyString matchgroup=rubyStringDelimit start="%r\[" end="\][iomx]*" skip="\\\\\|\\\]" contains=rubyExprSubst fold
syn region rubyString matchgroup=rubyStringDelimit start="%r("  end=")[iomx]*"  skip="\\\\\|\\)"  contains=rubyExprSubst fold

" Generalized Single Quoted String and Array of Strings
syn region rubyString matchgroup=rubyStringDelimit start="%[wq]!"  end="!"  skip="\\\\\|\\!"
syn region rubyString matchgroup=rubyStringDelimit start="%[wq]\"" end="\"" skip="\\\\\|\\\""
syn region rubyString matchgroup=rubyStringDelimit start="%[wq]#"  end="#"  skip="\\\\\|\\#"
syn region rubyString matchgroup=rubyStringDelimit start="%[wq]\$" end="\$" skip="\\\\\|\\\$"
syn region rubyString matchgroup=rubyStringDelimit start="%[wq]%"  end="%"  skip="\\\\\|\\%"
syn region rubyString matchgroup=rubyStringDelimit start="%[wq]&"  end="&"  skip="\\\\\|\\&"
syn region rubyString matchgroup=rubyStringDelimit start="%[wq]'"  end="'"  skip="\\\\\|\\'"
syn region rubyString matchgroup=rubyStringDelimit start="%[wq]\*" end="\*" skip="\\\\\|\\\*"
syn region rubyString matchgroup=rubyStringDelimit start="%[wq]+"  end="+"  skip="\\\\\|\\+"
syn region rubyString matchgroup=rubyStringDelimit start="%[wq]-"  end="-"  skip="\\\\\|\\-"
syn region rubyString matchgroup=rubyStringDelimit start="%[wq]\." end="\." skip="\\\\\|\\\."
syn region rubyString matchgroup=rubyStringDelimit start="%[wq]/"  end="/"  skip="\\\\\|\\/"
syn region rubyString matchgroup=rubyStringDelimit start="%[wq]:"  end=":"  skip="\\\\\|\\:"
syn region rubyString matchgroup=rubyStringDelimit start="%[wq];"  end=";"  skip="\\\\\|\\;"
syn region rubyString matchgroup=rubyStringDelimit start="%[wq]="  end="="  skip="\\\\\|\\="
syn region rubyString matchgroup=rubyStringDelimit start="%[wq]?"  end="?"  skip="\\\\\|\\?"
syn region rubyString matchgroup=rubyStringDelimit start="%[wq]@"  end="@"  skip="\\\\\|\\@"
syn region rubyString matchgroup=rubyStringDelimit start="%[wq]\\" end="\\"
syn region rubyString matchgroup=rubyStringDelimit start="%[wq]\^" end="\^" skip="\\\\\|\\\^"
syn region rubyString matchgroup=rubyStringDelimit start="%[wq]`"  end="`"  skip="\\\\\|\\`"
syn region rubyString matchgroup=rubyStringDelimit start="%[wq]|"  end="|"  skip="\\\\\|\\|"
syn region rubyString matchgroup=rubyStringDelimit start="%[wq]\~" end="\~" skip="\\\\\|\\\~"
syn region rubyString matchgroup=rubyStringDelimit start="%[wq]{"  end="}"  skip="\\\\\|\\}"
syn region rubyString matchgroup=rubyStringDelimit start="%[wq]<"  end=">"  skip="\\\\\|\\>"
syn region rubyString matchgroup=rubyStringDelimit start="%[wq]\[" end="\]" skip="\\\\\|\\\]"
syn region rubyString matchgroup=rubyStringDelimit start="%[wq]("  end=")"  skip="\\\\\|\\)"

" Generalized Double Quoted String and Shell Command Output
syn region rubyString matchgroup=rubyStringDelimit start="%[Qx]\=!"  end="!"  skip="\\\\\|\\!"  contains=rubyExprSubst
syn region rubyString matchgroup=rubyStringDelimit start="%[Qx]\=\"" end="\"" skip="\\\\\|\\\"" contains=rubyExprSubst
syn region rubyString matchgroup=rubyStringDelimit start="%[Qx]\=#"  end="#"  skip="\\\\\|\\#"  contains=rubyExprSubst
syn region rubyString matchgroup=rubyStringDelimit start="%[Qx]\=\$" end="\$" skip="\\\\\|\\\$" contains=rubyExprSubst
syn region rubyString matchgroup=rubyStringDelimit start="%[Qx]\=%"  end="%"  skip="\\\\\|\\%"  contains=rubyExprSubst
syn region rubyString matchgroup=rubyStringDelimit start="%[Qx]\=&"  end="&"  skip="\\\\\|\\&"  contains=rubyExprSubst
syn region rubyString matchgroup=rubyStringDelimit start="%[Qx]\='"  end="'"  skip="\\\\\|\\'"  contains=rubyExprSubst
syn region rubyString matchgroup=rubyStringDelimit start="%[Qx]\=\*" end="\*" skip="\\\\\|\\\*" contains=rubyExprSubst
syn region rubyString matchgroup=rubyStringDelimit start="%[Qx]\=+"  end="+"  skip="\\\\\|\\+"  contains=rubyExprSubst
syn region rubyString matchgroup=rubyStringDelimit start="%[Qx]\=-"  end="-"  skip="\\\\\|\\-"  contains=rubyExprSubst
syn region rubyString matchgroup=rubyStringDelimit start="%[Qx]\=\." end="\." skip="\\\\\|\\\." contains=rubyExprSubst
syn region rubyString matchgroup=rubyStringDelimit start="%[Qx]\=/"  end="/"  skip="\\\\\|\\/"  contains=rubyExprSubst
syn region rubyString matchgroup=rubyStringDelimit start="%[Qx]\=:"  end=":"  skip="\\\\\|\\:"  contains=rubyExprSubst
syn region rubyString matchgroup=rubyStringDelimit start="%[Qx]\=;"  end=";"  skip="\\\\\|\\;"  contains=rubyExprSubst
"syn region rubyString matchgroup=rubyStringDelimit start="%[Qx]\=="    end="="  skip="\\\\\|\\="  contains=rubyExprSubst
syn region rubyString matchgroup=rubyStringDelimit start="%[Qx]="    end="="  skip="\\\\\|\\="  contains=rubyExprSubst
syn region rubyString matchgroup=rubyStringDelimit start="%[Qx]\=?"  end="?"  skip="\\\\\|\\?"  contains=rubyExprSubst
syn region rubyString matchgroup=rubyStringDelimit start="%[Qx]\=@"  end="@"  skip="\\\\\|\\@"  contains=rubyExprSubst
syn region rubyString matchgroup=rubyStringDelimit start="%[Qx]\=\\" end="\\"			contains=rubyExprSubst
syn region rubyString matchgroup=rubyStringDelimit start="%[Qx]\=\^" end="\^" skip="\\\\\|\\\^" contains=rubyExprSubst
syn region rubyString matchgroup=rubyStringDelimit start="%[Qx]\=`"  end="`"  skip="\\\\\|\\`"  contains=rubyExprSubst
syn region rubyString matchgroup=rubyStringDelimit start="%[Qx]\=|"  end="|"  skip="\\\\\|\\|"  contains=rubyExprSubst
syn region rubyString matchgroup=rubyStringDelimit start="%[Qx]\=\~" end="\~" skip="\\\\\|\\\~" contains=rubyExprSubst
syn region rubyString matchgroup=rubyStringDelimit start="%[Qx]\={"  end="}"  skip="\\\\\|\\}"  contains=rubyExprSubst
syn region rubyString matchgroup=rubyStringDelimit start="%[Qx]\=<"  end=">"  skip="\\\\\|\\>"  contains=rubyExprSubst
syn region rubyString matchgroup=rubyStringDelimit start="%[Qx]\=\[" end="\]" skip="\\\\\|\\\]" contains=rubyExprSubst
syn region rubyString matchgroup=rubyStringDelimit start="%[Qx]\=("  end=")"  skip="\\\\\|\\)"  contains=rubyExprSubst

" Normal String and Shell Command Output
syn region rubyString matchgroup=rubyStringDelimit start="\"" end="\"" skip="\\\\\|\\\"" contains=rubyExprSubst
syn region rubyString matchgroup=rubyStringDelimit start="'"  end="'"  skip="\\\\\|\\'"
syn region rubyString matchgroup=rubyStringDelimit start="`"  end="`"  skip="\\\\\|\\`"  contains=rubyExprSubst
"
" END Autogenerated Stuff
"

" Normal Regular Expression
syn region rubyString matchgroup=rubyStringDelimit start="^\s*/" start="\<and\s*/"lc=3 start="\<or\s*/"lc=2 start="\<while\s*/"lc=5 start="\<until\s*/"lc=5 start="\<unless\s*/"lc=6 start="\<if\s*/"lc=2 start="\<elsif\s*/"lc=5 start="\<when\s*/"lc=4 start="[\~=!|&(,[]\s*/"lc=1 end="/[iomx]*" skip="\\\\\|\\/" contains=rubyExprSubst

" Here Document
if version < 600
  syn region rubyString matchgroup=rubyStringDelimit start=+<<-\(\u\{3,}\|'\u\{3,}'\|"\u\{3,}"\|`\u\{3,}`\)+hs=s+2 end=+^\s*\u\{3,}$+ fold
  syn region rubyString matchgroup=rubyStringDelimit start=+<<-\(EOF\|'EOF'\|"EOF"\|`EOF`\)+hs=s+2		   end=+^\s*EOF$+     contains=rubyExprSubst fold
  syn region rubyString matchgroup=rubyStringDelimit start=+<<-\(EOS\|'EOS'\|"EOS"\|`EOS`\)+hs=s+2		   end=+^\s*EOS$+     contains=rubyExprSubst fold
  syn region rubyString matchgroup=rubyStringDelimit start=+<<\(\u\{3,}\|'\u\{3,}'\|"\u\{3,}"\|`\u\{3,}`\)+hs=s+2  end=+^\u\{3,}$+    fold
  syn region rubyString matchgroup=rubyStringDelimit start=+<<\(EOF\|'EOF'\|"EOF"\|`EOF`\)+hs=s+2		   end=+^EOF$+	      contains=rubyExprSubst fold
  syn region rubyString matchgroup=rubyStringDelimit start=+<<\(EOS\|'EOS'\|"EOS"\|`EOS`\)+hs=s+2		   end=+^EOS$+	      contains=rubyExprSubst fold
else
 syn region rubyString matchgroup=rubyStringDelimit start=+\(class\s*\)\@<!<<\z(\h\w*\)\s*$+hs=s+2  end=+^\z1$+    contains=rubyExprSubst fold
 syn region rubyString matchgroup=rubyStringDelimit start=+\(class\s*\)\@<!<<"\z(.*\)"\s*$+hs=s+2   end=+^\z1$+    contains=rubyExprSubst fold
 syn region rubyString matchgroup=rubyStringDelimit start=+\(class\s*\)\@<!<<'\z(.*\)'\s*$+hs=s+2   end=+^\z1$+    fold
 syn region rubyString matchgroup=rubyStringDelimit start=+\(class\s*\)\@<!<<`\z(.*\)`\s*$+hs=s+2   end=+^\z1$+    contains=rubyExprSubst fold

 syn region rubyString matchgroup=rubyStringDelimit start=+\(class\s*\)\@<!<<-\z(\h\w*\)\s*$+hs=s+3 end=+^\s*\z1$+ contains=rubyExprSubst fold
 syn region rubyString matchgroup=rubyStringDelimit start=+\(class\s*\)\@<!<<-"\z(.*\)"\s*$+hs=s+3  end=+^\s*\z1$+ contains=rubyExprSubst fold
 syn region rubyString matchgroup=rubyStringDelimit start=+\(class\s*\)\@<!<<-'\z(.*\)'\s*$+hs=s+3  end=+^\s*\z1$+ fold
 syn region rubyString matchgroup=rubyStringDelimit start=+\(class\s*\)\@<!<<-`\z(.*\)`\s*$+hs=s+3  end=+^\s*\z1$+ contains=rubyExprSubst fold
endif

" Expensive Mode - colorize *end* according to opening statement
if !exists("ruby_no_expensive")
  syn region rubyFunction      matchgroup=rubyDefine start="^\s*def\s" matchgroup=NONE end="\ze\(\s\|(\|;\|$\)" skip="\.\|\(::\)" oneline fold
  syn region rubyClassOrModule matchgroup=rubyDefine start="^\s*\(class\|module\)\s"   end="<\|$\|;\|\>"he=e-1 oneline fold

  syn region rubyBlock start="^\s*def\s\+"rs=s		   matchgroup=rubyDefine end="\<end\>" contains=ALLBUT,rubyExprSubst,rubyTodo nextgroup=rubyFunction fold
  syn region rubyBlock start="^\s*\(class\|module\)\>"rs=s matchgroup=rubyDefine end="\<end\>" contains=ALLBUT,rubyExprSubst,rubyTodo nextgroup=rubyClassOrModule fold

  " modifiers + redundant *do*
  syn match  rubyControl "\<\(if\|unless\|while\|until\|do\)\>"

  " *do* requiring *end*
  syn region rubyDoBlock matchgroup=rubyControl start="\<do\>" end="\<end\>" contains=ALLBUT,rubyExprSubst,rubyTodo fold

  " *{* requiring *}*
  syn region rubyCurlyBlock start="{" end="}" contains=ALLBUT,rubyExprSubst,rubyTodo fold

  " statements without *do*
  syn region rubyNoDoBlock matchgroup=rubyControl start="\<\(case\|begin\)\>" start="^\s*\(if\|unless\)\>" start=";\s*\(if\|unless\)\>"hs=s+1 end="\<end\>" contains=ALLBUT,rubyExprSubst,rubyTodo fold

  " statement with optional *do*
  syn region rubyOptDoBlock matchgroup=rubyControl start="\<for\>" start="^\s*\(while\|until\)\>" start=";\s*\(while\|until\)\>"hs=s+1 end="\<end\>" contains=ALLBUT,rubyExprSubst,rubyTodo,rubyDoBlock,rubyCurlyBlock fold

  if !exists("ruby_minlines")
    let ruby_minlines = 50
  endif
  exec "syn sync minlines=" . ruby_minlines

else " not Expensive
  syn region  rubyFunction      matchgroup=rubyControl start="^\s*def\s" matchgroup=NONE end="\ze\(\s\|(\|;\|$\)" skip="\.\|\(::\)" oneline fold
  syn region  rubyClassOrModule matchgroup=rubyControl start="^\s*\(class\|module\)\s"   end="<\|$\|;\|\>"he=e-1 oneline fold
  syn keyword rubyControl case begin do for if unless while until end
endif " Expensive?

" Keywords
syn keyword rubyControl   then else elsif when ensure rescue
syn keyword rubyControl   and or not in loop
syn keyword rubyControl   break redo retry next return
syn match   rubyKeyword   "\<defined?"
syn keyword rubyKeyword   alias lambda proc super undef yield
syn match   rubyInclude   "^\s*include\>"
syn keyword rubyInclude   load require
syn keyword rubyTodo      FIXME NOTE TODO XXX contained
syn keyword rubyBoolean   true false self nil
syn keyword rubyException raise fail catch throw
syn keyword rubyBeginEnd  BEGIN END

" Comments and Documentation
if version < 600
  syn match  rubySharpBang "#!.*"
else
  syn match  rubySharpBang "\%^#!.*"
endif
syn match  rubyComment       "#.*" contains=rubyTodo
syn region rubyDocumentation start="^=begin" end="^=end.*$" contains=rubyTodo fold

" Note: this is a hack to prevent 'keywords' being highlighted as such when used as method names
syn match rubyKeywordAsMethod "\.\@<!\.\(\s*\n\s*\)*\(alias\|and\|begin\|break\|case\|catch\|class\|def\|do\|elsif\)\>"        transparent contains=NONE
syn match rubyKeywordAsMethod "\.\@<!\.\(\s*\n\s*\)*\(else\|fail\|false\|ensure\|for\|end\|if\|in\|include\|lambda\)\>"        transparent contains=NONE
syn match rubyKeywordAsMethod "\.\@<!\.\(\s*\n\s*\)*\(load\|loop\|module\|next\|nil\|not\|or\|proc\|raise\|require\)\>"        transparent contains=NONE
syn match rubyKeywordAsMethod "\.\@<!\.\(\s*\n\s*\)*\(redo\|rescue\|retry\|return\|self\|super\|then\|throw\|true\|unless\)\>" transparent contains=NONE
syn match rubyKeywordAsMethod "\.\@<!\.\(\s*\n\s*\)*\(undef\|until\|when\|while\|yield\|BEGIN\|END\|__FILE__\|__LINE__\)\>"    transparent contains=NONE

" __END__ Directive
syn region rubyData matchgroup=rubyDataDirective start="^__END__$" matchgroup=NONE end="." skip="."

" Define the default highlighting.
" For version 5.7 and earlier: only when not done already
" For version 5.8 and later: only when an item doesn't have highlighting yet
if version >= 508 || !exists("did_ruby_syntax_inits")
  if version < 508
    let did_ruby_syntax_inits = 1
    command -nargs=+ HiLink hi link <args>
  else
    command -nargs=+ HiLink hi def link <args>
  endif

  HiLink rubyDefine			Define
  HiLink rubyFunction			Function
  HiLink rubyControl			Statement
  HiLink rubyInclude			Include
  HiLink rubyNumber			Number
  HiLink rubyBoolean			Boolean
  HiLink rubyException			Exception
  HiLink rubyClassOrModule		Type
  HiLink rubyIdentifier			Identifier
  HiLink rubyClassVariable		rubyIdentifier
  HiLink rubyConstant			rubyIdentifier
  HiLink rubyGlobalVariable		rubyIdentifier
  HiLink rubyIterator			rubyIdentifier
  HiLink rubyInstanceVariable		rubyIdentifier
  HiLink rubyPredefinedIdentifier	rubyIdentifier
  HiLink rubyPredefinedConstant		rubyPredefinedIdentifier
  HiLink rubyPredefinedVariable		rubyPredefinedIdentifier
  HiLink rubySymbol			rubyIdentifier
  HiLink rubySharpBang			PreProc
  HiLink rubyKeyword			Keyword
  HiLink rubyBeginEnd			Statement

  HiLink rubyString			String
  HiLink rubyStringDelimit		Delimiter
  HiLink rubyExprSubst			Special

  HiLink rubyComment			Comment
  HiLink rubyDocumentation		Comment
  HiLink rubyTodo			Todo
  HiLink rubyData			Comment
  HiLink rubyDataDirective		Delimiter

  delcommand HiLink
endif

let b:current_syntax = "ruby"

" vim: nowrap tabstop=8

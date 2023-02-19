" Vim support file to detect file types
"
" Maintainer:	Bram Moolenaar <Bram@vim.org>
" Last Change:	2004 May 30

" Listen very carefully, I will say this only once
if exists("did_load_filetypes")
  finish
endif
let did_load_filetypes = 1

" Line continuation is used here, remove 'C' from 'cpoptions'
let s:cpo_save = &cpo
set cpo&vim

augroup filetypedetect

" Ignored extensions
au BufNewFile,BufRead *.orig,*.bak,*.old,*.new,*.rpmsave,*.rpmnew
	\ exe "doau filetypedetect BufRead " . expand("<afile>:r")
au BufNewFile,BufRead *~
	\ let s:name = expand("<afile>") |
	\ let s:short = substitute(s:name, '\~$', '', '') |
	\ if s:name != s:short && s:short != "" |
	\   exe "doau filetypedetect BufRead " . s:short |
	\ endif |
	\ unlet s:name |
	\ unlet s:short
au BufNewFile,BufRead *.in
	\ if expand("<afile>:t") != "configure.in" |
	\   exe "doau filetypedetect BufRead " . expand("<afile>:r") |
	\ endif

" Pattern used to match file names which should not be inspected.
" Currently finds compressed files.
if !exists("g:ft_ignore_pat")
  let g:ft_ignore_pat = '\.\(Z\|gz\|bz2\|zip\|tgz\)$'
endif

" Abaqus or Trasys
au BufNewFile,BufRead *.inp			call FTCheck_inp()

fun! FTCheck_inp()
  if getline(1) =~ '^\*'
    setf abaqus
  else
    let n = 1
    if line("$") > 500
      let nmax = 500
    else
      let nmax = line("$")
    endif
    while n <= nmax
      if getline(n) =~? "^header surface data"
	setf trasys
	break
      endif
      let n = n + 1
    endwhile
  endif
endfun

" A-A-P recipe
au BufNewFile,BufRead *.aap			setf aap

" ABC music notation
au BufNewFile,BufRead *.abc			setf abc

" ABEL
au BufNewFile,BufRead *.abl			setf abel

" AceDB
au BufNewFile,BufRead *.wrm			setf acedb

" Ada (83, 9X, 95)
au BufNewFile,BufRead *.adb,*.ads,*.ada		setf ada

" AHDL
au BufNewFile,BufRead *.tdf			setf ahdl

" AMPL
au BufNewFile,BufRead *.run			setf ampl

" Ant
au BufNewFile,BufRead build.xml			setf ant

" Apache style config file
au BufNewFile,BufRead proftpd.conf*		setf apachestyle

" Apache config file
au BufNewFile,BufRead httpd.conf*,srm.conf*,access.conf*,.htaccess,apache.conf* setf apache

" XA65 MOS6510 cross assembler
au BufNewFile,BufRead *.a65			setf a65

" Applix ELF
au BufNewFile,BufRead *.am
	\ if expand("<afile>") !~? 'Makefile.am\>' | setf elf | endif

" Arc Macro Language
au BufNewFile,BufRead *.aml			setf aml

" Arch Inventory file
au BufNewFile,BufRead .arch-inventory,=tagging-method	setf arch

" ART*Enterprise (formerly ART-IM)
au BufNewFile,BufRead *.art			setf art

" ASN.1
au BufNewFile,BufRead *.asn,*.asn1		setf asn

" Active Server Pages (with Visual Basic Script)
au BufNewFile,BufRead *.asa
	\ if exists("g:filetype_asa") |
	\   exe "setf " . g:filetype_asa |
	\ else |
	\   setf aspvbs |
	\ endif

" Active Server Pages (with Perl or Visual Basic Script)
au BufNewFile,BufRead *.asp
	\ if exists("g:filetype_asp") |
	\   exe "setf " . g:filetype_asp |
	\ elseif getline(1) . getline(2) . getline(3) =~? "perlscript" |
	\   setf aspperl |
	\ else |
	\   setf aspvbs |
	\ endif

" Grub (must be before catch *.lst)
au BufNewFile,BufRead /boot/grub/menu.lst,/boot/grub/grub.conf	setf grub

" Assembly (all kinds)
" *.lst is not pure assembly, it has two extra columns (address, byte codes)
au BufNewFile,BufRead *.asm,*.[sS],*.[aA],*.mac,*.lst	call <SID>FTasm()

" This function checks for the kind of assembly that is wanted by the user, or
" can be detected from the first five lines of the file.
fun! <SID>FTasm()
  " make sure b:asmsyntax exists
  if !exists("b:asmsyntax")
    let b:asmsyntax = ""
  endif

  if b:asmsyntax == ""
    call FTCheck_asmsyntax()
  endif

  " if b:asmsyntax still isn't set, default to asmsyntax or GNU
  if b:asmsyntax == ""
    if exists("g:asmsyntax")
      let b:asmsyntax = g:asmsyntax
    else
      let b:asmsyntax = "asm"
    endif
  endif

  exe "setf " . b:asmsyntax
endfun

fun! FTCheck_asmsyntax()
  " see if file contains any asmsyntax=foo overrides. If so, change
  " b:asmsyntax appropriately
  let head = " ".getline(1)." ".getline(2)." ".getline(3)." ".getline(4).
	\" ".getline(5)." "
  if head =~ '\sasmsyntax=\S\+\s'
    let b:asmsyntax = substitute(head, '.*\sasmsyntax=\(\S\+\)\s.*','\1', "")
  elseif ((head =~? '\.title') || (head =~? '\.ident') || (head =~? '\.macro') || (head =~? '\.subtitle') || (head =~? '\.library'))
    let b:asmsyntax = "vmasm"
  endif
endfun

" Macro (VAX)
au BufNewFile,BufRead *.mar			setf vmasm

" Atlas
au BufNewFile,BufRead *.atl,*.as		setf atlas

" Automake
au BufNewFile,BufRead [mM]akefile.am		setf automake

" Avenue
au BufNewFile,BufRead *.ave			setf ave

" Awk
au BufNewFile,BufRead *.awk			setf awk

" B
au BufNewFile,BufRead *.mch,*.ref,*.imp		setf b

" BASIC or Visual Basic
au BufNewFile,BufRead *.bas			call <SID>FTVB("basic")

" Check if one of the first five lines contains "VB_Name".  In that case it is
" probably a Visual Basic file.  Otherwise it's assumed to be "alt" filetype.
fun! <SID>FTVB(alt)
  if getline(1).getline(2).getline(3).getline(4).getline(5) =~? 'VB_Name\|Begin VB\.\(Form\|MDIForm\|UserControl\)'
    setf vb
  else
    exe "setf " . a:alt
  endif
endfun

" Visual Basic Script (close to Visual Basic)
au BufNewFile,BufRead *.vbs,*.dsm,*.ctl		setf vb

" Batch file for MSDOS.
au BufNewFile,BufRead *.bat,*.btm,*.sys		setf dosbatch
" *.cmd is close to a Batch file, but on OS/2 Rexx files also use *.cmd.
au BufNewFile,BufRead *.cmd
	\ if getline(1) =~ '^/\*' | setf rexx | else | setf dosbatch | endif

" Batch file for 4DOS
au BufNewFile,BufRead *.btm			setf btm

" BC calculator
au BufNewFile,BufRead *.bc			setf bc

" BDF font
au BufNewFile,BufRead *.bdf			setf bdf

" BibTeX bibliography database file
au BufNewFile,BufRead *.bib			setf bib

" BIND configuration
au BufNewFile,BufRead named.conf		setf named

" BIND zone
au BufNewFile,BufRead named.root		setf bindzone

" Blank
au BufNewFile,BufRead *.bl			setf blank

" C or lpc
au BufNewFile,BufRead *.c			call <SID>FTlpc()

fun! <SID>FTlpc()
  if exists("g:lpc_syntax_for_c")
    let lnum = 1
    while lnum <= 12
      if getline(lnum) =~# '^\(//\|inherit\|private\|protected\|nosave\|string\|object\|mapping\|mixed\)'
	setf lpc
	return
      endif
      let lnum = lnum + 1
    endwhile
  endif
  setf c
endfun

" Calendar
au BufNewFile,BufRead calendar,~/.calendar/*,
	\*/share/calendar/*/calendar.*,*/share/calendar/calendar.*
	\					setf calendar

" C#
au BufNewFile,BufRead *.cs			setf cs

" Comshare Dimension Definition Language
au BufNewFile,BufRead *.cdl			setf cdl

" Controllable Regex Mutilator
au BufNewFile,BufRead *.crm			setf crm

" Cyn++
au BufNewFile,BufRead *.cyn			setf cynpp

" Cynlib
" .cc and .cpp files can be C++ or Cynlib.
au BufNewFile,BufRead *.cc
	\ if exists("cynlib_syntax_for_cc")|setf cynlib|else|setf cpp|endif
au BufNewFile,BufRead *.cpp
	\ if exists("cynlib_syntax_for_cpp")|setf cynlib|else|setf cpp|endif

" C++
if has("fname_case")
  au BufNewFile,BufRead *.cxx,*.c++,*.C,*.H,*.hh,*.hxx,*.hpp,*.moc,*.tcc,*.inl setf cpp
else
  au BufNewFile,BufRead *.cxx,*.c++,*.hh,*.hxx,*.hpp,*.moc,*.tcc,*.inl setf cpp
endif

" .h files can be C, Ch or C++, set c_syntax_for_h if you want C,
" ch_syntax_for_h if you want Ch.
au BufNewFile,BufRead *.h
	\ if exists("c_syntax_for_h") | setf c |
	\ elseif exists("ch_syntax_for_h") | setf ch |
	\ else | setf cpp | endif

" Ch (CHscript)
au BufNewFile,BufRead *.chf			setf ch

" TLH files are C++ headers generated by Visual C++'s #import from typelibs
au BufNewFile,BufRead *.tlh			setf cpp

" Cascading Style Sheets
au BufNewFile,BufRead *.css			setf css

" Century Term Command Scripts (*.cmd too)
au BufNewFile,BufRead *.con			setf cterm

" Changelog
au BufNewFile,BufRead changelog.Debian,changelog.dch setf debchangelog
au BufNewFile,BufRead [cC]hange[lL]og		if getline(1) =~ '; urgency='
	\| setf debchangelog | else | setf changelog | endif

" CHILL
au BufNewFile,BufRead *..ch			setf chill

" Changes for WEB and CWEB or CHILL
au BufNewFile,BufRead *.ch			call <SID>FTchange()

" This function checks if one of the first ten lines start with a '@'.  In
" that case it is probably a change file.
" If the first line starts with # or ! it's probably a ch file.
" If a line has "main", "include", "//" ir "/*" it's probably ch.
" Otherwise CHILL is assumed.
fun! <SID>FTchange()
  let lnum = 1
  while lnum <= 10
    if getline(lnum)[0] == '@'
      setf change
      return
    endif
    if lnum == 1 && (getline(1)[0] == '#' || getline(1)[0] == '!')
      setf ch
      return
    endif
    if getline(lnum) =~ "MODULE"
      setf chill
      return
    endif
    if getline(lnum) =~ 'main\s*(\|#\s*include\|//'
      setf ch
      return
    endif
    let lnum = lnum + 1
  endwhile
  setf chill
endfun

" Clean
au BufNewFile,BufRead *.dcl,*.icl		setf clean

" Clever
au BufNewFile,BufRead *.eni			setf cl

" Clever or dtd
au BufNewFile,BufRead *.ent			call <SID>FTent()

fun! <SID>FTent()
  " This function checks for valid cl syntax in the first five lines.
  " Look for either an opening comment, '#', or a block start, '{".
  " If not found, assume SGML.
  let lnum = 1
  while lnum < 6
    let line = getline(lnum)
    if line =~ '^\s*[#{]'
      setf cl
      return
    elseif line !~ '^\s*$'
      " Not a blank line, not a comment, and not a block start,
      " so doesn't look like valid cl code.
      break
    endif
    let lnum = lnum + 1
  endw
  setf dtd
endfun

" Clipper (or FoxPro)
au BufNewFile,BufRead *.prg
	\ if exists("g:filetype_prg") |
	\   exe "setf " . g:filetype_prg |
	\ else |
	\   setf clipper |
	\ endif

" Cobol
au BufNewFile,BufRead *.cbl,*.cob,*.cpy,*.lib	setf cobol

" Cold Fusion
au BufNewFile,BufRead *.cfm,*.cfi,*.cfc		setf cf

" Configure scripts
au BufNewFile,BufRead configure.in,configure.ac setf config

" WildPackets EtherPeek Decoder
au BufNewFile,BufRead *.dcd			setf dcd

" Enlightenment configuration files
au BufNewFile,BufRead *enlightenment/*.cfg	setf c

" Eterm
au BufNewFile,BufRead *Eterm/*.cfg		setf eterm

" Lynx config files
au BufNewFile,BufRead lynx.cfg			setf lynx

" Quake
au BufNewFile,BufRead *baseq[2-3]/*.cfg,*id1/*.cfg	setf quake
au BufNewFile,BufRead *quake[1-3]/*.cfg			setf quake

" Quake C
au BufNewFile,BufRead *.qc			setf c

" Configure files
au BufNewFile,BufRead *.cfg			setf cfg

" Communicating Sequential Processes
au BufNewFile,BufRead *.csp,*.fdr		setf csp

" CUPL logic description and simulation
au BufNewFile,BufRead *.pld			setf cupl
au BufNewFile,BufRead *.si			setf cuplsim

" Debian Control
au BufNewFile,BufRead */debian/control		setf debcontrol

" ROCKLinux package description
au BufNewFile,BufRead *.desc			setf desc

" the D language
au BufNewFile,BufRead *.d			setf d

" Desktop files
au BufNewFile,BufRead *.desktop,.directory	setf desktop

" Diff files
au BufNewFile,BufRead *.diff,*.rej,*.patch	setf diff

" Dircolors
au BufNewFile,BufRead .dir_colors,/etc/DIR_COLORS	setf dircolors

" Diva (with Skill) or InstallShield
au BufNewFile,BufRead *.rul
	\ if getline(1).getline(2).getline(3).getline(4).getline(5).getline(6) =~? 'InstallShield' |
	\   setf ishd |
	\ else |
	\   setf diva |
	\ endif

" DCL (Digital Command Language - vms) or DNS zone file
au BufNewFile,BufRead *.com
	\ if getline(1).getline(2) =~ '$ORIGIN\|$TTL\|IN\s*SOA'
	\	|| getline(1).getline(2).getline(3).getline(4) =~ 'BIND.*named'
	\ | setf dns | else | setf dcl | endif

" DOT
au BufNewFile,BufRead *.dot			setf dot

" Dylan - lid files
au BufNewFile,BufRead *.lid			setf dylanlid

" Dylan - intr files (melange)
au BufNewFile,BufRead *.intr			setf dylanintr

" Dylan
au BufNewFile,BufRead *.dylan			setf dylan

" Microsoft Module Definition
au BufNewFile,BufRead *.def			setf def

" Dracula
au BufNewFile,BufRead *.drac,*.drc,*lvs,*lpe	setf dracula

" dsl
au BufNewFile,BufRead *.dsl			setf dsl

" DTD (Document Type Definition for XML)
au BufNewFile,BufRead *.dtd			setf dtd

" EDIF (*.edf,*.edif,*.edn,*.edo)
au BufNewFile,BufRead *.ed\(f\|if\|n\|o\)	setf edif

" Embedix Component Description
au BufNewFile,BufRead *.ecd			setf ecd

" Eiffel or Specman
au BufNewFile,BufRead *.e,*.E			call FTCheck_e()

" Elinks configuration
au BufNewFile,BufRead */etc/elinks.conf,~/.elinks/elinks.conf	setf elinks

fun! FTCheck_e()
  let n = 1
  while n < 100 && n < line("$")
    if getline(n) =~ "^\\s*\\(<'\\|'>\\)\\s*$"
      setf specman
      return
    endif
    let n = n + 1
  endwhile
  setf eiffel
endfun

" ERicsson LANGuage
au BufNewFile,BufRead *.erl			setf erlang

" Elm Filter Rules file
au BufNewFile,BufRead filter-rules		setf elmfilt

" ESQL-C
au BufNewFile,BufRead *.ec,*.EC			setf esqlc

" Essbase script
au BufNewFile,BufRead *.csc			setf csc

" Exim
au BufNewFile,BufRead exim.conf			setf exim

" Expect
au BufNewFile,BufRead *.exp			setf expect

" Exports
au BufNewFile,BufRead exports			setf exports

" Fetchmail RC file
au BufNewFile,BufRead .fetchmailrc		setf fetchmail

" Focus Executable
au BufNewFile,BufRead *.fex,*.focexec		setf focexec

" Focus Master file (but not for auto.master)
au BufNewFile,BufRead auto.master		setf conf
au BufNewFile,BufRead *.mas,*.master		setf master

" Forth
au BufNewFile,BufRead *.fs,*.ft			setf forth

" Fortran
au BufNewFile,BufRead *.f,*.F,*.for,*.fpp,*.ftn,*.f77,*.F77,*.f90,*.F90,*.f95,*.F95	setf fortran

" FStab
au BufNewFile,BufRead fstab			setf fstab

" GDB command files
au BufNewFile,BufRead .gdbinit			setf gdb

" GDMO
au BufNewFile,BufRead *.mo,*.gdmo		setf gdmo

" Gedcom
au BufNewFile,BufRead *.ged			setf gedcom

" Gkrellmrc
au BufNewFile,BufRead gkrellmrc,gkrellmrc_?	setf gkrellmrc

" GP scripts (2.0 and onward)
au BufNewFile,BufRead *.gp			setf gp

" GPG
au BufNewFile,BufRead ~/.gnupg/options		setf gpg
au BufNewFile,BufRead ~/.gnupg/gpg.conf		setf gpg
au BufNewFile,BufRead /usr/**/gnupg/options.skel setf gpg

" Gnuplot scripts
au BufNewFile,BufRead *.gpi			setf gnuplot

" GrADS scripts
au BufNewFile,BufRead *.gs			setf grads

" Groovy
au BufNewFile,BufRead *.groovy			setf groovy

" GNU Server Pages
au BufNewFile,BufRead *.gsp			setf gsp

" GTK RC
au BufNewFile,BufRead .gtkrc,gtkrc		setf gtkrc

" Haskell
au BufNewFile,BufRead *.hs			setf haskell
au BufNewFile,BufRead *.lhs			setf lhaskell
au BufNewFile,BufRead *.chs			setf chaskell

" Hercules
au BufNewFile,BufRead *.vc,*.ev,*.rs,*.sum,*.errsum	setf hercules

" HEX (Intel)
au BufNewFile,BufRead *.hex,*.h32		setf hex

" Tilde (must be before HTML)
au BufNewFile,BufRead *.t.html			setf tilde

" HTML (.shtml and .stm for server side)
au BufNewFile,BufRead *.html,*.htm,*.shtml,*.stm  call <SID>FTCheck_html()

" Distinguish between HTML and XHTML
fun! <SID>FTCheck_html()
  let n = 1
  while n < 10 && n < line("$")
    if getline(n) =~ '\<DTD\s\+XHTML\s'
      setf xhtml
      return
    endif
    let n = n + 1
  endwhile
  setf html
endfun


" HTML with M4
au BufNewFile,BufRead *.html.m4			setf htmlm4

" HTML Cheetah template
au BufNewFile,BufRead *.tmpl			setf htmlcheetah

" Hyper Builder
au BufNewFile,BufRead *.hb			setf hb

" Icon
au BufNewFile,BufRead *.icn			setf icon

" IDL (Interface Description Language)
au BufNewFile,BufRead *.idl			call <SID>FTCheck_idl()

" Distinguish between standard IDL and MS-IDL
fun! <SID>FTCheck_idl()
  let n = 1
  while n < 50 && n < line("$")
    if getline(n) =~ '^\s*import\s\+"\(unknwn\|objidl\)\.idl"'
      setf msidl
      return
    endif
    let n = n + 1
  endwhile
  setf idl
endfun

" Microsoft IDL (Interface Description Language)  Also *.idl
" MOF = WMI (Windows Management Instrumentation) Managed Object Format
au BufNewFile,BufRead *.odl,*.mof		setf msidl

" Icewm menu
au BufNewFile,BufRead ~/.icewm/menu		setf icemenu

" Inform
au BufNewFile,BufRead .indent.pro		setf indent

" IDL (Interactive Data Language)
au BufNewFile,BufRead *.pro			setf idlang

" Inform
au BufNewFile,BufRead *.inf,*.INF		setf inform

" Informix 4GL (source - canonical, include file, I4GL+M4 preproc.)
au BufNewFile,BufRead *.4gl,*.4gh,*.m4gl	setf fgl

" .INI file for MSDOS
au BufNewFile,BufRead *.ini			setf dosini

" SysV Inittab
au BufNewFile,BufRead inittab			setf inittab

" Inno Setup
au BufNewFile,BufRead *.iss			setf iss

" JAL
au BufNewFile,BufRead *.jal,*.JAL		setf jal

" Jam
au BufNewFile,BufRead *.jpl,*.jpr		setf jam

" Java
au BufNewFile,BufRead *.java,*.jav		setf java

" JavaCC
au BufNewFile,BufRead *.jj,*.jjt		setf javacc

" JavaScript
au BufNewFile,BufRead *.js,*.javascript		setf javascript

" Java Server Pages
au BufNewFile,BufRead *.jsp			setf jsp

" Java Properties resource file (note: doesn't catch font.properties.pl)
au BufNewFile,BufRead *.properties,*.properties_??,*.properties_??_??,*.properties_??_??_*	setf jproperties

" Jess
au BufNewFile,BufRead *.clp			setf jess

" Jgraph
au BufNewFile,BufRead *.jgr			setf jgraph

" Kixtart
au BufNewFile,BufRead *.kix			setf kix

" Kimwitu[++]
au BufNewFile,BufRead *.k			setf kwt

" KDE script
au BufNewFile,BufRead *.ks			setf kscript

" Lace (ISE)
au BufNewFile,BufRead *.ace,*.ACE		setf lace

" Latte
au BufNewFile,BufRead *.latte,*.lte		setf latte

" LambdaProlog (*.mod too, see Modsim)
au BufNewFile,BufRead *.sig			setf lprolog

" LDAP LDIF
au BufNewFile,BufRead *.ldif			setf ldif

" Lex
au BufNewFile,BufRead *.lex,*.l			setf lex

" Libao
au BufNewFile,BufRead /etc/libao.conf,~/.libao	setf libao

" LFTP
au BufNewFile,BufRead lftp.conf,.lftprc,*lftp/rc	setf lftp

" Lifelines (or Lex for C++!)
au BufNewFile,BufRead *.ll			setf lifelines

" Lilo: Linux loader
au BufNewFile,BufRead lilo.conf*		setf lilo

" Lisp (*.el = ELisp, *.cl = Common Lisp, *.jl = librep Lisp)
if has("fname_case")
  au BufNewFile,BufRead *.lsp,*.lisp,*.el,*.cl,*.jl,*.L,.emacs,.sawfishrc setf lisp
else
  au BufNewFile,BufRead *.lsp,*.lisp,*.el,*.cl,*.jl,.emacs,.sawfishrc setf lisp
endif

" Lite
au BufNewFile,BufRead *.lite,*.lt		setf lite

" Logtalk
au BufNewFile,BufRead *.lgt			setf logtalk

" LOTOS
au BufNewFile,BufRead *.lot,*.lotos		setf lotos

" Lout (also: *.lt)
au BufNewFile,BufRead *.lou,*.lout		setf lout

" Lua
au BufNewFile,BufRead *.lua			setf lua

" Lynx style file (or LotusScript!)
au BufNewFile,BufRead *.lss			setf lss

" M4
au BufNewFile,BufRead *.m4
	\ if expand("<afile>") !~? 'html.m4$\|fvwm2rc' | setf m4 | endif

" MaGic Point
au BufNewFile,BufRead *.mgp			setf mgp

" Mail (for Elm, trn, mutt, rn, slrn)
au BufNewFile,BufRead snd.\d\+,.letter,.letter.\d\+,.followup,.article,.article.\d\+,pico.\d\+,mutt-*-\w\+,mutt\w\{6\},ae\d\+.txt,/tmp/SLRN[0-9A-Z.]\+,*.eml setf mail

" Mailcap configuration file
au BufNewFile,BufRead .mailcap,mailcap		setf mailcap

" Makefile
au BufNewFile,BufRead *[mM]akefile,*.mk,*.mak,*.dsp setf make

" MakeIndex
au BufNewFile,BufRead *.ist,*.mst		setf ist

" Manpage
au BufNewFile,BufRead *.man			setf man

" Maple V
au BufNewFile,BufRead *.mv,*.mpl,*.mws		setf maple

" Mason
au BufNewFile,BufRead *.mason,*.mhtml		setf mason

" Matlab or Objective C
au BufNewFile,BufRead *.m			call FTCheck_m()

fun! FTCheck_m()
  let n = 1
  while n < 10
    let line = getline(n)
    if line =~ '^\s*\(#\s*\(include\|import\)\>\|/\*\)'
      setf objc
      return
    endif
    if line =~ '^\s*%'
      setf matlab
      return
    endif
    if line =~ '^\s*(\*'
      setf mma
      return
    endif
    let n = n + 1
  endwhile
  setf matlab
endfun

" Maya Extension Language
au BufNewFile,BufRead *.mel			setf mel

" Metafont
au BufNewFile,BufRead *.mf			setf mf

" MetaPost
au BufNewFile,BufRead *.mp			setf mp

" MMIX or VMS makefile
au BufNewFile,BufRead *.mms			call FTCheck_mms()

fun! FTCheck_mms()
  let n = 1
  while n < 10
    let line = getline(n)
    if line =~ '^\s*\(%\|//\)' || line =~ '^\*'
      setf mmix
      return
    endif
    if line =~ '^\s*#'
      setf make
      return
    endif
    let n = n + 1
  endwhile
  setf mmix
endfun


" Modsim III (or LambdaProlog)
au BufNewFile,BufRead *.mod
	\ if getline(1) =~ '\<module\>' |
	\   setf lprolog |
	\ else |
	\   setf modsim3 |
	\ endif

" Modula 2
au BufNewFile,BufRead *.m2,*.DEF,*.MOD,*.md,*.mi setf modula2

" Modula 3 (.m3, .i3, .mg, .ig)
au BufNewFile,BufRead *.[mi][3g]		setf modula3

" Monk
au BufNewFile,BufRead *.isc,*.monk,*.ssc,*.tsc	setf monk

" MOO
au BufNewFile,BufRead *.moo			setf moo

" Modconf
au BufNewFile,BufRead /etc/modules.conf,/etc/conf.modules	setf modconf
au BufNewFile,BufRead /etc/modutils/*
	\ if executable(expand("<afile>")) != 1 | setf modconf | endif

" Mplayer config
au BufNewFile,BufRead mplayer.conf,~/.mplayer/config	setf mplayerconf

" Moterola S record
au BufNewFile,BufRead *.s19,*.s28,*.s37		setf srec

" Msql
au BufNewFile,BufRead *.msql			setf msql

" Mysql
au BufNewFile,BufRead *.mysql			setf mysql

" M$ Resource files
au BufNewFile,BufRead *.rc			setf rc

" Mush
au BufNewFile,BufRead *.mush			setf mush

" Mutt setup file
au BufNewFile,BufRead .muttrc*,~/.mutt/muttrc*,Muttrc	setf muttrc

" Nastran input/DMAP
"au BufNewFile,BufRead *.dat			setf nastran

" Natural
au BufNewFile,BufRead *.NS[ACGLMNPS]		setf natural

" Novell netware batch files
au BufNewFile,BufRead *.ncf			setf ncf

" Nroff/Troff (*.ms and *.t are checked below)
au BufNewFile,BufRead *.me
	\ if expand("<afile>") != "read.me" && expand("<afile>") != "click.me" |
	\   setf nroff |
	\ endif
au BufNewFile,BufRead *.tr,*.nr,*.roff,*.tmac,*.mom	setf nroff
au BufNewFile,BufRead *.[1-9]			call <SID>FTnroff()

" This function checks if one of the first five lines start with a dot.  In
" that case it is probably an nroff file: 'filetype' is set and 1 is returned.
fun! <SID>FTnroff()
  if getline(1)[0] . getline(2)[0] . getline(3)[0] . getline(4)[0] . getline(5)[0] =~ '\.'
    setf nroff
    return 1
  endif
  return 0
endfun

" Nroff or Objective C++
au BufNewFile,BufRead *.mm			call <SID>FTcheck_mm()

fun! <SID>FTcheck_mm()
  let n = 1
  while n < 10
    let line = getline(n)
    if line =~ '^\s*\(#\s*\(include\|import\)\>\|/\*\)'
      setf objcpp
      return
    endif
    let n = n + 1
  endwhile
  setf nroff
endfun

" Not Quite C
au BufNewFile,BufRead *.nqc			setf nqc

" NSIS
au BufNewFile,BufRead *.nsi			setf nsis

" OCAML
au BufNewFile,BufRead *.ml,*.mli,*.mll,*.mly	setf ocaml

" Occam
au BufNewFile,BufRead *.occ			setf occam

" Omnimark
au BufNewFile,BufRead *.xom,*.xin		setf omnimark

" OpenROAD
au BufNewFile,BufRead *.or			setf openroad

" OPL
au BufNewFile,BufRead *.[Oo][Pp][Ll]		setf opl

" Oracle config file
au BufNewFile,BufRead *.ora			setf ora

" Packet filter conf
au BufNewFile,BufRead pf.conf			setf pf

" PApp
au BufNewFile,BufRead *.papp,*.pxml,*.pxsl	setf papp

" Pascal (also *.p)
au BufNewFile,BufRead *.pas			setf pascal

" Delphi project file
au BufNewFile,BufRead *.dpr			setf pascal

" Perl
if has("fname_case")
  au BufNewFile,BufRead *.pl,*.PL		call FTCheck_pl()
else
  au BufNewFile,BufRead *.pl			call FTCheck_pl()
endif

fun! FTCheck_pl()
  if exists("g:filetype_pl")
    exe "setf " . g:filetype_pl
  else
    " recognize Prolog by specific text in the first non-empty line
    " require a blank after the '%' because Perl uses "%list" and "%translate"
    let l = getline(nextnonblank(1))
    if l =~ '\<prolog\>' || l =~ '^\s*\(%\+\(\s\|$\)\|/\*\)' || l =~ ':-'
      setf prolog
    else
      setf perl
    endif
  endif
endfun

" Perl, XPM or XPM2
au BufNewFile,BufRead *.pm
	\ if getline(1) =~ "XPM2" |
	\   setf xpm2 |
	\ elseif getline(1) =~ "XPM" |
	\   setf xpm |
	\ else |
	\   setf perl |
	\ endif

" Perl POD
au BufNewFile,BufRead *.pod			setf pod

" Php3
au BufNewFile,BufRead *.php,*.php3		setf php

" Phtml
au BufNewFile,BufRead *.phtml			setf phtml

" Pike
au BufNewFile,BufRead *.pike,*.lpc,*.ulpc,*.pmod setf pike

" Pinfo config
au BufNewFile,BufRead */etc/pinforc,~/.pinforc	setf pinfo

" Palm Resource compiler
au BufNewFile,BufRead *.rcp			setf pilrc

" Pine config
au BufNewFile,BufRead .pinerc,pinerc,.pinercex,pinercex		setf pine

" PL/M (also: *.inp)
au BufNewFile,BufRead *.plm,*.p36,*.pac		setf plm

" PL/SQL
au BufNewFile,BufRead *.pls,*.plsql		setf plsql

" PLP
au BufNewFile,BufRead *.plp			setf plp

" PO and PO template (GNU gettext)
au BufNewFile,BufRead *.po,*.pot		setf po

" Postfix main config
au BufNewFile,BufRead main.cf			setf pfmain

" PostScript (+ font files, encapsulated PostScript, Adobe Illustrator)
au BufNewFile,BufRead *.ps,*.pfa,*.afm,*.eps,*.epsf,*.epsi,*.ai	  setf postscr

" PostScript Printer Description
au BufNewFile,BufRead *.ppd			setf ppd

" Povray
au BufNewFile,BufRead *.pov			setf pov

" Povray configuration
au BufNewFile,BufRead .povrayrc			setf povini

" Povray, PHP or assembly
au BufNewFile,BufRead *.inc			call FTCheck_inc()

fun! FTCheck_inc()
  if exists("g:filetype_inc")
    exe "setf " . g:filetype_inc
  else
    let lines = getline(1).getline(2).getline(3)
    if lines =~? "perlscript"
      setf aspperl
    elseif lines =~ "<%"
      setf aspvbs
    elseif lines =~ "<?"
      setf php
    else
      call FTCheck_asmsyntax()
      if exists("b:asmsyntax")
	exe "setf " . b:asmsyntax
      else
	setf pov
      endif
    endif
  endif
endfun

" Printcap and Termcap
au BufNewFile,BufRead *printcap
	\ let b:ptcap_type = "print" | setf ptcap
au BufNewFile,BufRead *termcap
	\ let b:ptcap_type = "term" | setf ptcap

" PCCTS / ANTRL
"au BufNewFile,BufRead *.g			setf antrl
au BufNewFile,BufRead *.g			setf pccts

" PPWizard
au BufNewFile,BufRead *.it,*.ih			setf ppwiz

" Oracle Pro*C/C++
au BufNewFile,BufRead .pc			setf proc

" Procmail
au BufNewFile,BufRead .procmail,.procmailrc	setf procmail

" Progress or CWEB
au BufNewFile,BufRead *.w			call <SID>FTprogress_cweb()

function! <SID>FTprogress_cweb()
  if exists("g:filetype_w")
    exe "setf " . g:filetype_w
    return
  endif
  if getline(1) =~ '&ANALYZE' || getline(3) =~ '&GLOBAL-DEFINE'
    setf progress
  else
    setf cweb
  endif
endfun

" Progress or assembly
au BufNewFile,BufRead *.i			call <SID>FTprogress_asm()

function! <SID>FTprogress_asm()
  if exists("g:filetype_i")
    exe "setf " . g:filetype_i
    return
  endif
  " This function checks for an assembly comment the first ten lines.
  " If not found, assume Progress.
  let lnum = 1
  while lnum <= 10
    let line = getline(lnum)
    if line =~ '^\s*;' || line =~ '^\*'
      call FTCheck_asm()
      return
    elseif line !~ '^\s*$' || line =~ '^/\*'
      " Not an empty line: Doesn't look like valid assembly code.
      " Or it looks like a Progress /* comment
      break
    endif
    let lnum = lnum + 1
  endw
  setf progress
endfun

" Progress or Pascal
au BufNewFile,BufRead *.p			call <SID>FTprogress_pascal()

function! <SID>FTprogress_pascal()
  if exists("g:filetype_p")
    exe "setf " . g:filetype_p
    return
  endif
  " This function checks for valid Pascal syntax in the first ten lines.
  " Look for either an opening comment or a program start.
  " If not found, assume Progress.
  let lnum = 1
  while lnum <= 10
    let line = getline(lnum)
    if line =~ '^\s*\(program\|procedure\|function\|const\|type\|var\)\>'
	\ || line =~ '^\s*{' || line =~ '^\s*(\*'
      setf pascal
      return
    elseif line !~ '^\s*$' || line =~ '^/\*'
      " Not an empty line: Doesn't look like valid Pascal code.
      " Or it looks like a Progress /* comment
      break
    endif
    let lnum = lnum + 1
  endw
  setf progress
endfun


" Software Distributor Product Specification File (POSIX 1387.2-1995)
au BufNewFile,BufRead *.psf			setf psf
au BufNewFile,BufRead INDEX,INFO
	\ if getline(1) =~ '^\s*\(distribution\|installed_software\|root\|bundle\|product\)\s*$' |
	\   setf psf |
	\ endif

" Prolog
au BufNewFile,BufRead *.pdb			setf prolog

" Pyrex
au BufNewFile,BufRead *.pyx,*.pxd		setf pyrex

" Python
au BufNewFile,BufRead *.py,*.pyw		setf python

" Radiance
au BufNewFile,BufRead *.rad,*.mat		setf radiance

" Ratpoison config/command files
au BufNewFile,BufRead .ratpoisonrc,ratpoisonrc	setf ratpoison

" RCS file
au BufNewFile,BufRead *\,v			setf rcs

" Readline
au BufNewFile,BufRead .inputrc			setf readline

" Registry for MS-Windows
au BufNewFile,BufRead *.reg
	\ if getline(1) =~? '^REGEDIT[0-9]*\s*$\|^Windows Registry Editor Version \d*\.\d*\s*$' | setf registry | endif

" Renderman Interface Bytestream
au BufNewFile,BufRead *.rib			setf rib

" Rexx
au BufNewFile,BufRead *.rexx,*.rex		setf rexx

" R (Splus)
au BufNewFile,BufRead *.s,*.S			setf r

" Rexx, Rebol or R
au BufNewFile,BufRead *.r,*.R			call <SID>FTCheck_r()

fun! <SID>FTCheck_r()
  if getline(1) =~ '^REBOL'
    setf rebol
  else
    let n = 1
    let max = line("$")
    if max > 50
      let max = 50
    endif
    while n < max
      " R has # comments
      if getline(n) =~ '^\s*#'
	setf r
	break
      endif
      " Rexx has /* comments */
      if getline(n) =~ '^\s*/\*'
	setf rexx
	break
      endif
      let n = n + 1
    endwhile
    if n >= max
      setf rexx
    endif
  endif
endfun

" Remind
au BufNewFile,BufRead .reminders*		setf remind

" Resolv.conf
au BufNewFile,BufRead resolv.conf		setf resolv

" Relax NG Compact
au BufNewFile,BufRead *.rnc			setf rnc

" RPL/2
au BufNewFile,BufRead *.rpl			setf rpl

" Robots.txt
au BufNewFile,BufRead robots.txt		setf robots

" Rpcgen
au BufNewFile,BufRead *.x			setf rpcgen

" reStructuredText Documentation Format
au BufNewFile,BufRead *.rst			setf rst

" RTF
au BufNewFile,BufRead *.rtf			setf rtf

" Ruby
au BufNewFile,BufRead *.rb,*.rbw,*.gem,*.gemspec	setf ruby

" S-lang (or shader language!)
au BufNewFile,BufRead *.sl			setf slang

" Samba config
au BufNewFile,BufRead smb.conf			setf samba

" SAS script
au BufNewFile,BufRead *.sas			setf sas

" Sather
au BufNewFile,BufRead *.sa			setf sather

" Scilab
au BufNewFile,BufRead *.sci			setf scilab

" SDL
au BufNewFile,BufRead *.sdl,*.pr		setf sdl

" sed
au BufNewFile,BufRead *.sed			setf sed

" Sendmail
au BufNewFile,BufRead sendmail.cf		setf sm

" Sendmail .mc files are actually m4
au BufNewFile,BufRead *.mc			setf m4

" SGML
au BufNewFile,BufRead *.sgm,*.sgml
	\ if getline(1).getline(2).getline(3).getline(4).getline(5) =~? 'linuxdoc' |
	\   setf sgmllnx |
	\ elseif getline(1) =~ '<!DOCTYPE.*DocBook' || getline(2) =~ '<!DOCTYPE.*DocBook' |
	\   let b:docbk_type="sgml" |
	\   setf docbk |
	\ else |
	\   setf sgml |
	\ endif

" SGMLDECL
au BufNewFile,BufRead *.decl,*.dcl,*.dec
	\ if getline(1).getline(2).getline(3) =~? '^<!SGML' |
	\    setf sgmldecl |
	\ endif

" SGML catalog file
au BufNewFile,BufRead sgml.catalog*,catalog	setf catalog

" Shell scripts (sh, ksh, bash, bash2, csh); Allow .profile_foo etc.
" Gentoo ebuilds are actually bash scripts
au BufNewFile,BufRead .bashrc*,bashrc,bash.bashrc,.bash_profile*,.bash_logout*,*.bash,*.ebuild call SetFileTypeSH("bash")
au BufNewFile,BufRead .kshrc*,*.ksh call SetFileTypeSH("ksh")
au BufNewFile,BufRead /etc/profile,.profile*,*.sh,*.env call SetFileTypeSH(getline(1))

fun! SetFileTypeSH(name)
  if a:name =~ '\<ksh\>'
    let b:is_kornshell = 1
    if exists("b:is_bash")
      unlet b:is_bash
    endif
    if exists("b:is_sh")
      unlet b:is_sh
    endif
  elseif exists("g:bash_is_sh") || a:name =~ '\<bash\>' || a:name =~ '\<bash2\>'
    let b:is_bash = 1
    if exists("b:is_kornshell")
      unlet b:is_kornshell
    endif
    if exists("b:is_sh")
      unlet b:is_sh
    endif
  elseif a:name =~ '\<sh\>'
    let b:is_sh = 1
    if exists("b:is_kornshell")
      unlet b:is_kornshell
    endif
    if exists("b:is_bash")
      unlet b:is_bash
    endif
  endif
  setf sh
endfun

" tcsh scripts
au BufNewFile,BufRead .tcshrc*,*.tcsh,tcsh.tcshrc,tcsh.login	setf tcsh

" csh scripts, but might also be tcsh scripts (on some systems csh is tcsh)
au BufNewFile,BufRead .login*,.cshrc*,csh.cshrc,csh.login,csh.logout,*.csh,.alias  call SetFileTypeCSH()

fun! SetFileTypeCSH()
  if exists("g:filetype_csh")
    exe "setf " . g:filetype_csh
  elseif &shell =~ "tcsh"
    setf tcsh
  else
    setf csh
  endif
endfun

" Z-Shell script
au BufNewFile,BufRead .zsh*,.zlog*,.zprofile,/etc/zprofile,.zfbfmarks,.zcompdump*  setf zsh

" Scheme
au BufNewFile,BufRead *.scm,*.ss		setf scheme

" Screen RC
au BufNewFile,BufRead .screenrc,screenrc	setf screen

" Simula
au BufNewFile,BufRead *.sim			setf simula

" SINDA
au BufNewFile,BufRead *.sin,*.s85		setf sinda

" SKILL
au BufNewFile,BufRead *.il			setf skill

" SLRN
au BufNewFile,BufRead .slrnrc			setf slrnrc
au BufNewFile,BufRead *.score			setf slrnsc

" Smalltalk
au BufNewFile,BufRead *.st,*.cls		setf st

" Smarty templates
au BufNewFile,BufRead *.tpl			setf smarty

" SMIL or XML
au BufNewFile,BufRead *.smil
	\ if getline(1) =~ '<?\s*xml.*?>' |
	\   setf xml |
	\ else |
	\   setf smil |
	\ endif

" SMIL or SNMP MIB file
au BufNewFile,BufRead *.smi
	\ if getline(1) =~ '\<smil\>' |
	\   setf smil |
	\ else |
	\   setf mib |
	\ endif

" SMITH
au BufNewFile,BufRead *.smt,*.smith		setf smith

" Snobol4
au BufNewFile,BufRead *.sno			setf snobol4

" SNMP MIB files
au BufNewFile,BufRead *.mib,*.my		setf mib

" Snort Configuration
au BufNewFile,BufRead *.hog,snort.conf,vision.conf,*.rules	setf hog

" Spec (Linux RPM)
au BufNewFile,BufRead *.spec			setf spec

" Speedup (AspenTech plant simulator)
au BufNewFile,BufRead *.speedup,*.spdata,*.spd	setf spup

" Slice
au BufNewFile,BufRead *.ice			setf slice

" Spice
au BufNewFile,BufRead *.sp,*.spice		setf spice

" Spyce
au BufNewFile,BufRead *.spy,*.spi		setf spyce

" Squid
au BufNewFile,BufRead squid.conf		setf squid

" SQL (all but the first one for Oracle Designer)
au BufNewFile,BufRead *.sql,*.tyb,*.typ,*.tyc,*.pkb,*.pks	setf sql

" SQLJ
au BufNewFile,BufRead *.sqlj			setf sqlj

" SQR
au BufNewFile,BufRead *.sqr,*.sqi		setf sqr

" OpenSSH configuration
au BufNewFile,BufRead ssh_config,.ssh/config	setf sshconfig

" OpenSSH server configuration
au BufNewFile,BufRead sshd_config		setf sshdconfig

" Stored Procedures
au BufNewFile,BufRead *.stp			setf stp

" Standard ML
au BufNewFile,BufRead *.sml			setf sml

" Tads (or Nroff)
au BufNewFile,BufRead *.t
	\ if !<SID>FTnroff() | setf tads | endif

" Tags
au BufNewFile,BufRead tags			setf tags

" TAK
au BufNewFile,BufRead *.tak			setf tak

" Tcl
au BufNewFile,BufRead *.tcl,*.tk,*.itcl,*.itk	setf tcl

" TealInfo
au BufNewFile,BufRead *.tli			setf tli

" Telix Salt
au BufNewFile,BufRead *.slt			setf tsalt

" Terminfo
au BufNewFile,BufRead *.ti			setf terminfo

" TeX
au BufNewFile,BufRead *.tex,*.latex,*.sty,*.dtx,*.ltx,*.bbl	setf tex

" Texinfo
au BufNewFile,BufRead *.texinfo,*.texi,*.txi	setf texinfo

" TeX configuration
au BufNewFile,BufRead texmf.cnf			setf texmf

" Tidy config
au BufNewFile,BufRead .tidyrc,tidyrc		setf tidy

" TF mud client
au BufNewFile,BufRead *.tf,.tfrc,tfrc		setf tf

" TSS - Geometry
au BufNewFile,BufReadPost *.tssgm		setf tssgm

" TSS - Optics
au BufNewFile,BufReadPost *.tssop		setf tssop

" TSS - Command Line (temporary)
au BufNewFile,BufReadPost *.tsscl		setf tsscl

" Motif UIT/UIL files
au BufNewFile,BufRead *.uit,*.uil		setf uil

" UnrealScript
au BufNewFile,BufRead *.uc			setf uc

" Verilog HDL
au BufNewFile,BufRead *.v			setf verilog

" VHDL
au BufNewFile,BufRead *.hdl,*.vhd,*.vhdl,*.vhdl_[0-9]*,*.vbe,*.vst  setf vhdl

" Vim script
au BufNewFile,BufRead *.vim,.exrc,_exrc		setf vim

" Viminfo file
au BufNewFile,BufRead .viminfo,_viminfo		setf viminfo

" Virata Config Script File
au BufRead,BufNewFile *.hw,*.module,*.pkg	setf virata

" Visual Basic (also uses *.bas) or FORM
au BufNewFile,BufRead *.frm			call <SID>FTVB("form")

" SaxBasic is close to Visual Basic
au BufNewFile,BufRead *.sba			setf vb

" Vgrindefs file
au BufNewFile,BufRead vgrindefs			setf vgrindefs

" VRML V1.0c
au BufNewFile,BufRead *.wrl			setf vrml

" Webmacro
au BufNewFile,BufRead *.wm			setf webmacro

" Wget config
au BufNewFile,BufRead .wgetrc,wgetrc		setf wget

" Website MetaLanguage
au BufNewFile,BufRead *.wml			setf wml

" Winbatch
au BufNewFile,BufRead *.wbt			setf winbatch

" WvDial
au BufNewFile,BufRead wvdial.conf,.wvdialrc	setf wvdial

" CVS RC file
au BufNewFile,BufRead .cvsrc			setf cvsrc

" CVS commit file
au BufNewFile,BufRead cvs\d\+			setf cvs

" WEB (*.web is also used for Winbatch: Guess, based on expecting "%" comment
" lines in a WEB file).
au BufNewFile,BufRead *.web
	\ if getline(1)[0].getline(2)[0].getline(3)[0].getline(4)[0].getline(5)[0] =~ "%" |
	\   setf web |
	\ else |
	\   setf winbatch |
	\ endif

" Windows Scripting Host and Windows Script Component
au BufNewFile,BufRead *.ws[fc]			setf wsh

" X Pixmap (dynamically sets colors, use BufEnter to make it work better)
au BufEnter *.xpm
	\ if getline(1) =~ "XPM2" |
	\   setf xpm2 |
	\ else |
	\   setf xpm |
	\ endif
au BufEnter *.xpm2				setf xpm2

" XFree86 config
au BufNewFile,BufRead XF86Config
	\ if getline(1) =~ '\<XConfigurator\>' |
	\   let b:xf86c_xfree86_version = 3 |
	\ endif |
	\ setf xf86conf

" Xorg config
au BufNewFile,BufRead xorg.conf,xorg.conf-4	let b:xf86c_xfree86_version = 4 | setf xf86conf

" XS Perl extension interface language
au BufNewFile,BufRead *.xs			setf xs

" X resources file
au BufNewFile,BufRead .Xdefaults,.Xpdefaults,.Xresources,xdm-config,*.ad setf xdefaults

" Xmath
au BufNewFile,BufRead *.msc,*.msf		setf xmath
au BufNewFile,BufRead *.ms
	\ if !<SID>FTnroff() | setf xmath | endif

" XML
au BufNewFile,BufRead *.xml
	\ if getline(1) . getline(2) . getline(3) =~ '<!DOCTYPE.*DocBook' |
	\   let b:docbk_type="xml" |
	\   setf docbk |
	\ else |
	\   setf xml |
	\ endif

" XMI (holding UML models) is also XML
au BufNewFile,BufRead *.xmi			setf xml

" CSPROJ files are Visual Studio.NET's XML-based project config files
au BufNewFile,BufRead *.csproj,*.csproj.user	setf xml

" Qt Linguist translation source and Qt User Interface Files are XML
au BufNewFile,BufRead *.ts,*.ui			setf xml

" XSD
au BufNewFile,BufRead *.xsd			setf xsd

" Xslt
au BufNewFile,BufRead *.xsl,*.xslt		setf xslt

" Yacc
au BufNewFile,BufRead *.y,*.yy			setf yacc

" Yaml
au BufNewFile,BufRead *.yaml,*.yml		setf yaml

" Z80 assembler asz80
au BufNewFile,BufRead *.z8a			setf z8a

augroup END


" Source the user-specified filetype file, for backwards compatibility with
" Vim 5.x.
if exists("myfiletypefile") && file_readable(expand(myfiletypefile))
  execute "source " . myfiletypefile
endif


" Check for "*" after loading myfiletypefile, so that scripts.vim is only used
" when there are no matching file name extensions.
" Don't do this for compressed files.
augroup filetypedetect
au BufNewFile,BufRead *
	\ if !did_filetype() && expand("<amatch>") !~ g:ft_ignore_pat
	\ | runtime! scripts.vim | endif
au StdinReadPost * if !did_filetype() | runtime! scripts.vim | endif


" Extra checks for when no filetype has been detected now.  Mostly used for
" patterns that end in "*".  E.g., "zsh*" matches "zsh.vim", but that's a Vim
" script file.

" BIND zone
au BufNewFile,BufRead /var/named/*		setf bindzone

" Changelog
au BufNewFile,BufRead [cC]hange[lL]og*		if getline(1) =~ '; urgency='
	\| setf debchangelog | else | setf changelog | endif

" Crontab
au BufNewFile,BufRead crontab,crontab.*		setf crontab

" Dracula
au BufNewFile,BufRead drac.*			setf dracula

" Fvwm
au BufNewFile,BufRead *fvwmrc*,*fvwm95*.hook
	\ let b:fvwm_version = 1 | setf fvwm
au BufNewFile,BufRead *fvwm2rc*
	\ if expand("<afile>:e") == "m4" | setf fvwm2m4 | else |
	\ let b:fvwm_version = 2 | setf fvwm | endif

" GTK RC
au BufNewFile,BufRead .gtkrc*,gtkrc*		setf gtkrc

" Jam
au BufNewFile,BufRead Prl*.*,JAM*.*		setf jam

" Jargon
au! BufNewFile,BufRead *jarg*
	\ if getline(1).getline(2).getline(3).getline(4).getline(5) =~? 'THIS IS THE JARGON FILE' |
	\   setf jargon |
	\ endif

" Makefile
au BufNewFile,BufRead [mM]akefile*		setf make

" Ruby Makefile
au BufNewFile,BufRead [rR]akefile*		setf ruby

" Mutt setup file
au BufNewFile,BufRead muttrc*,Muttrc*		setf muttrc

" Nroff macros
au BufNewFile,BufRead tmac.*			setf nroff

" Printcap and Termcap
au BufNewFile,BufRead *printcap*
	\ if !did_filetype() | let b:ptcap_type = "print" | setf ptcap | endif
au BufNewFile,BufRead *termcap*
	\ if !did_filetype() | let b:ptcap_type = "term" | setf ptcap | endif

" Vim script
au BufNewFile,BufRead *vimrc*			setf vim

" Subversion commit file
au BufNewFile,BufRead svn-commit.*.tmp		setf svn

" X resources file
au BufNewFile,BufRead Xresources*,*/app-defaults/*,*/Xresources/* setf xdefaults

" XFree86 config
au BufNewFile,BufRead XF86Config-4*
	\ let b:xf86c_xfree86_version = 4 | setf xf86conf
au BufNewFile,BufRead XF86Config*
	\ if getline(1) =~ '\<XConfigurator\>' |
	\   let b:xf86c_xfree86_version = 3 |
	\ endif |
	\ setf xf86conf

" X11 xmodmap
au BufNewFile,BufRead *xmodmap*			setf xmodmap

" Z-Shell script
au BufNewFile,BufRead zsh*,zlog*		setf zsh


" Generic configuration file (check this last, it's just guessing!)
au BufNewFile,BufRead,StdinReadPost *
	\ if !did_filetype() && expand("<amatch>") !~ g:ft_ignore_pat
	\    && (getline(1) =~ '^#' || getline(2) =~ '^#' || getline(3) =~ '^#'
	\	|| getline(4) =~ '^#' || getline(5) =~ '^#') |
	\   setf conf |
	\ endif

" Use the plugin-filetype checks last, they may overrule any of the previously
" detected filetypes.
runtime! ftdetect/*.vim

augroup END


" If the GUI is already running, may still need to install the Syntax menu.
" Don't do it when the 'M' flag is included in 'guioptions'.
if has("menu") && has("gui_running")
      \ && !exists("did_install_syntax_menu") && &guioptions !~# "M"
  source <sfile>:p:h/menu.vim
endif

" Restore 'cpoptions'
let &cpo = s:cpo_save
unlet s:cpo_save

" Menu Translations:	Simplified Chinese (UNIX)
" Translated By:	Wang Jun <junw@turbolinux.com.cn>
" Last Change:		Tue Sep  4 11:26:52 CST 2001

" Quit when menu translations have already been done.
if exists("did_menu_trans")
  finish
endif
let did_menu_trans = 1

scriptencoding gb2312

" Help menu
menutrans &Help			����(&H)
menutrans &Overview<Tab><F1>	Ԥ��(&O)<Tab><F1>
menutrans &User\ Manual		�û��ֲ�(&U)
menutrans &GUI			ͼ�ν���(&G)
menutrans &How-to\ links	HOWTO�ĵ�\.\.\.(&H)
menutrans &Credits		����(&C)
menutrans Co&pying		��Ȩ(&P)
menutrans &Version		�汾(&V)
menutrans &About		����\ Vim(&A)

" File menu
menutrans &File				�ļ�(&F)
menutrans &Open\.\.\.<Tab>:e		��(&O)\.\.\.<Tab>:e
menutrans Sp&lit-Open\.\.\.<Tab>:sp	�ָ�ڲ���(&L)<Tab>:sp
menutrans &New<Tab>:enew		�½�(&N)<Tab>:enew
menutrans &Close<Tab>:close		�ر�(&C)<Tab>:close
menutrans &Save<Tab>:w			����(&S)<Tab>:w
menutrans Save\ &As\.\.\.<Tab>:sav	���Ϊ(&A)\.\.\.<Tab>:sav
menutrans Split\ &Diff\ with\.\.\.	�ָ�Ƚ�(&Diff)\.\.\.
menutrans Split\ Patched\ &By\.\.\.	�ָ�򲹶�(&Patch)\.\.\.
menutrans &Print			��ӡ(&P)
menutrans Sa&ve-Exit<Tab>:wqa		���沢�˳�(&V)<Tab>:wqa
menutrans E&xit<Tab>:qa			�˳�(&X)<Tab>:qa

" Edit menu
menutrans &Edit				�༭(&E)
menutrans &Undo<Tab>u			�ָ�(&U)<Tab>u
menutrans &Redo<Tab>^R			����(&R)<Tab>^R
menutrans Rep&eat<Tab>\.		�ظ��ϴζ���(&E)<Tab>\.
menutrans Cu&t<Tab>"+x			����(&T)<Tab>"+x
menutrans &Copy<Tab>"+y			����(&C)<Tab>"+y
menutrans &Paste<Tab>"+gP		ճ��(&P)<Tab>"+gP
menutrans Put\ &Before<Tab>[p		�������ǰ(&B)<Tab>[p
menutrans Put\ &After<Tab>]p		��������(&A)<Tab>]p
menutrans &Delete<Tab>x			ɾ��(&D)<Tab>x
menutrans &Select\ all<Tab>ggVG		ȫѡ(&S)<Tab>ggvG
menutrans &Find\.\.\.			����(&F)\.\.\.
menutrans Find\ and\ Rep&lace\.\.\.	�����滻(&L)\.\.\.
menutrans Settings\ &Window		�趨����(&W)
menutrans &Global\ Settings		ȫ���趨(&G)

" Build boolean options
menutrans Toggle\ Pattern\ &Highlight<Tab>:set\ hls!	��/����������ģʽ<Tab>:set\ hls!
menutrans Toggle\ &Ignore-case<Tab>:set\ ic!		��/�غ��Դ�Сдģʽ<Tab>:set\ ic!
menutrans Toggle\ &Showmatch<Tab>:set\ sm!		��/��ƥ����ʾ<Tab>:set sm!
menutrans &Context\ lines			����������(&C)

menutrans &Virtual\ Edit			���ӻ��༭ģʽ(&V)
menutrans Never					�Ӳ�
menutrans Block\ Selection			��ѡ��
menutrans Insert\ mode				����ģʽ
menutrans Block\ and\ Insert			��ѡ�������ģʽ
menutrans Always				����ģʽ

menutrans Toggle\ Insert\ &Mode<Tab>:set\ im!	��/�ز���ģʽ<Tab>:set\ im!

menutrans Search\ &Path\.\.\.			����·��\.\.\.(&P)

menutrans Ta&g\ Files\.\.\.			��ǩ�ļ�\.\.\.(&g)

" GUI options
menutrans Toggle\ &Toolbar			��/�ع�����(&T)
menutrans Toggle\ &Bottom\ Scrollbar		��/�صײ�������(&B)
menutrans Toggle\ &Left\ Scrollbar		��/����˹�����(&L)
menutrans Toggle\ &Right\ Scrollbar		��/���Ҷ˹�����(&R)


" Edit/File Settings
menutrans F&ile\ Settings			�ļ��趨(&i)

" Boolean options
menutrans Toggle\ Line\ &Numbering<Tab>:set\ nu!	��/����ʾ�к�<Tab>:set\ nu!
menutrans Toggle\ &List\ Mode<Tab>:set\ list!		��/����ʾTab<Tab>:set\ list!
menutrans Toggle\ Line\ &Wrap<Tab>:set\ wrap!		��/���Զ�����<Tab>:set\ wrap!
menutrans Toggle\ W&rap\ at\ word<Tab>:set\ lbr!	��/�ش�β����<Tab>:set\ lbr!
menutrans Toggle\ &expand-tab<Tab>:set\ et!		��/��expand-tab<Tab>:set\ et!
menutrans Toggle\ &auto-indent<Tab>:set\ ai!		��/��auto-indent<Tab>:set\ ai!
menutrans Toggle\ &C-indenting<Tab>:set\ cin!		��/��C-indent<Tab>:set\ cin!


" other options
menutrans &Shiftwidth			���ſ��(&S)
menutrans Soft\ &Tabstop		αTab���(&T)
menutrans Te&xt\ Width\.\.\.		ҳ����(&x)\.\.\.
menutrans &File\ Format\.\.\.		�ļ���ʽ(&F)\.\.\.

menutrans C&olor\ Scheme		��ɫ��(&o)
menutrans Select\ Fo&nt\.\.\.		ѡ������(&n)\.\.\.


" Programming menu
menutrans &Tools			����(&T)
menutrans &Jump\ to\ this\ tag<Tab>g^]	������괦�ı�ǩ�ؼ���(tag)(&J)<Tab>g^]
menutrans Jump\ &back<Tab>^T		���ؼ���ǰ��λ��(&B)<Tab>^T
menutrans Build\ &Tags\ File		������ǩ�����ļ�\ Tags(&T)
menutrans &Folding			Folding�趨(&F)
menutrans &Diff				�Ƚ�(&D)
menutrans &Make<Tab>:make		ִ��\ Make(&M)<Tab>:make
menutrans &List\ Errors<Tab>:cl		�г��������(&E)<Tab>:cl
menutrans L&ist\ Messages<Tab>:cl!	�г�������Ϣ(&I)<Tab>:cl!
menutrans &Next\ Error<Tab>:cn		��һ���������(&N)<Tab>:cn
menutrans &Previous\ Error<Tab>:cp	��һ���������(&P)<Tab>:cp
menutrans &Older\ List<Tab>:cold	�ɴ����б�(&O)<Tab>:cold
menutrans N&ewer\ List<Tab>:cnew	�´����б�(&E)<Tab>:cnew
menutrans Error\ &Window		������Ϣ����(&W)
menutrans &Set\ Compiler		���ñ�����(&S)
menutrans &Convert\ to\ HEX<Tab>:%!xxd	ת����16����<Tab>:%!xxd
menutrans Conve&rt\ back<Tab>:%!xxd\ -r	��16����ת��������<Tab>:%!xxd\ -r

" Tools.Fold Menu
menutrans &Enable/Disable\ folds<Tab>zi		ʹ��/��ʹ��Folding(&E)<Tab>zi
menutrans &View\ Cursor\ Line<Tab>zv		�鿴����(&V)<Tab>zv
menutrans Vie&w\ Cursor\ Line\ only<Tab>zMzx	ֻ�鿴����(&W)<Tab>zMzx
menutrans C&lose\ more\ folds<Tab>zm		�ر�Folds(&L)<Tab>zm
menutrans &Close\ all\ folds<Tab>zM		�ر�����Folds(&C)<Tab>zM
menutrans O&pen\ more\ folds<Tab>zr		չ��Folds(&P)<Tab>zr
menutrans &Open\ all\ folds<Tab>zR		չ������Folds(&O)<Tab>zR
" fold method
menutrans Fold\ Met&hod				Fold��ʽ(&H)
menutrans Create\ &Fold<Tab>zf			����Fold(&F)<Tab>zf
menutrans &Delete\ Fold<Tab>zd			ɾ��Fold(&D)<Tab>zd
menutrans Delete\ &All\ Folds<Tab>zD		ɾ������Fold(&A)<Tab>zD
" moving around in folds
menutrans Fold\ column\ &width			�趨Fold����(&W)

" Tools.Diff Menu
menutrans &Update		����(&U)
menutrans &Get\ Block		ȡ�ò�ͬ����(&G)
menutrans &Put\ Block		����ͬ����Ӧ�õ��Է�(&P)


" Names for buffer menu.
menutrans &Buffers		������(&B)
menutrans &Refresh\ menu	����(&R)
menutrans &Delete		ɾ��(&D)
menutrans &Alternate		�޸�(&L)
menutrans &Next			��һ��(&N)
menutrans &Previous		ǰһ��(&P)

" Window menu
menutrans &Window			����(&W)
menutrans &New<Tab>^Wn			�½�����(&N)<Tab>^Wn
menutrans S&plit<Tab>^Ws		�ָ��(&P)<Tab>^Ws
menutrans Sp&lit\ To\ #<Tab>^W^^	�ָ#(&L)<Tab>^W^^
menutrans Split\ &Vertically<Tab>^Wv	��ֱ�ָ�(&V)<Tab>^Wv
menutrans Split\ File\ E&xplorer	�ļ������ʽ�ָ�(&X)
menutrans &Close<Tab>^Wc		�رմ���(&C)<Tab>^Wc
menutrans Close\ &Other(s)<Tab>^Wo	�ر���������(&O)<Tab>^Wo
menutrans Move\ &To			�ƶ���(&T)
menutrans &Top<Tab>^WK			����(&T)<Tab>^WK
menutrans &Bottom<Tab>^WJ		�׶�(&B)<Tab>^WJ
menutrans &Left\ side<Tab>^WH		���(&L)<Tab>^WH
menutrans &Right\ side<Tab>^WL		�ұ�(&R)<Tab>^WL
" menutrans Ne&xt<Tab>^Ww		��һ��(&X)<Tab>^Ww
" menutrans P&revious<Tab>^WW		��һ��(&R)<Tab>^WW
menutrans Rotate\ &Up<Tab>^WR		���ƴ���(&U)<Tab>^WR
menutrans Rotate\ &Down<Tab>^Wr		���ƴ���(&D)<Tab>^Wr
menutrans &Equal\ Size<Tab>^W=		���д��ڵȸ�(&E)<Tab>^W=
menutrans &Max\ Height<Tab>^W_		���߶�(&M)<Tab>^W
menutrans M&in\ Height<Tab>^W1_		��С�߶�(&i)<Tab>^W1_
menutrans Max\ &Width<Tab>^W\|		�����(&W)<Tab>^W\|
menutrans Min\ Widt&h<Tab>^W1\|		��С���(&h)<Tab>^W1\|
"
" The popup menu
menutrans &Undo			����(&U)
menutrans Cu&t			����(&T)
menutrans &Copy			����(&C)
menutrans &Paste		ճ��(&P)
menutrans &Delete		ɾ��(&D)
menutrans Select\ Blockwise	Blockwiseѡ��
menutrans Select\ &Word		ѡ�񵥴�(&W)
menutrans Select\ &Line		ѡ����(&L)
menutrans Select\ &Block	ѡ���(&B)
menutrans Select\ &All		ȫѡ(&A)
"
" The GUI toolbar
if has("toolbar")
  if exists("*Do_toolbar_tmenu")
    delfun Do_toolbar_tmenu
  endif
  fun Do_toolbar_tmenu()
    tmenu ToolBar.Open		���ļ�
    tmenu ToolBar.Save		���浱ǰ�ļ�
    tmenu ToolBar.SaveAll	����ȫ���ļ�
    tmenu ToolBar.Print		��ӡ
    tmenu ToolBar.Undo		�����ϴ��޸�
    tmenu ToolBar.Redo		�����ϴγ����Ķ���
    tmenu ToolBar.Cut		������������
    tmenu ToolBar.Copy		���Ƶ�������
    tmenu ToolBar.Paste		�ɼ�����ճ��
    tmenu ToolBar.Find		����...
    tmenu ToolBar.FindNext	������һ��
    tmenu ToolBar.FindPrev	������һ��
    tmenu ToolBar.Replace	�滻...
    tmenu ToolBar.LoadSesn	���ػỰ
    tmenu ToolBar.SaveSesn	���浱ǰ�ĻỰ
    tmenu ToolBar.RunScript	����Vim�ű�
    tmenu ToolBar.Make		ִ�� Make
    tmenu ToolBar.Shell		��һ�������
    tmenu ToolBar.RunCtags	ִ�� ctags
    tmenu ToolBar.TagJump	������ǰ���λ�õı�ǩ
    tmenu ToolBar.Help		Vim ����
    tmenu ToolBar.FindHelp	���� Vim ����
  endfun
endif

" Syntax menu
menutrans &Syntax		�﷨(&S)
menutrans Set\ '&syntax'\ only	ֻ�趨\ 'syntax'(&s)
menutrans Set\ '&filetype'\ too	Ҳ�趨\ 'filetype'(&f)
menutrans &Off			�ر�(&O)
menutrans &Manual		�ֶ��趨(&M)
menutrans A&utomatic		�Զ��趨(&U)
menutrans on/off\ for\ &This\ file	ֻ������ļ���/�ر�(&T)
menutrans Co&lor\ test		ɫ����ʾ����(&L)
menutrans &Highlight\ test	�﷨Ч������(&H)
menutrans &Convert\ to\ HTML	ת����\ HTML\ ��ʽ(&C)

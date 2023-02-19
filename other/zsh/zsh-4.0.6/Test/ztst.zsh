#!/usr/local/bin/zsh -f
# The line above is just for convenience.  Normally tests will be run using
# a specified version of zsh.  With dynamic loading, any required libraries
# must already have been installed in that case.
#
# Takes one argument: the name of the test file.  Currently only one such
# file will be processed each time ztst.zsh is run.  This is slower, but
# much safer in terms of preserving the correct status.
# To avoid namespace pollution, all functions and parameters used
# only by the script begin with ZTST_.
#
# Options (without arguments) may precede the test file argument; these
# are interpreted as shell options to set.  -x is probably the most useful.

# Produce verbose messages if non-zero.
# If 1, produce reports of tests executed; if 2, also report on progress.
# Defined in such a way that any value from the environment is used.
: ${ZTST_verbose:=0}

# We require all options to be reset, not just emulation options.
# Unfortunately, due to the crud which may be in /etc/zshenv this might
# still not be good enough.  Maybe we should trick it somehow.
emulate -R zsh

# Ensure the locale does not screw up sorting.  Don't supply a locale
# unless there's one set, to minimise problems.
[[ -n $LC_ALL ]] && LC_ALL=C
[[ -n $LC_COLLATE ]] && LC_COLLATE=C
[[ -n $LANG ]] && LANG=C

# Set the module load path to correspond to this build of zsh.
# This Modules directory should have been created by "make check".
[[ -d Modules/zsh ]] && module_path=( $PWD/Modules )

# We need to be able to save and restore the options used in the test.
# We use the $options variable of the parameter module for this.
zmodload -i zsh/parameter

# Note that both the following are regular arrays, since we only use them
# in whole array assignments to/from $options.
# Options set in test code (i.e. by default all standard options)
ZTST_testopts=(${(kv)options})

setopt extendedglob nonomatch
while [[ $1 = [-+]* ]]; do
  set $1
  shift
done
# Options set in main script
ZTST_mainopts=(${(kv)options})

# We run in the current directory, so remember it.
ZTST_testdir=$PWD
ZTST_testname=$1

integer ZTST_testfailed

# The source directory is not necessarily the current directory,
# but if $0 doesn't contain a `/' assume it is.
if [[ $0 = */* ]]; then
  ZTST_srcdir=${0%/*}
else
  ZTST_srcdir=$PWD
fi
[[ $ZTST_srcdir = /* ]] || ZTST_srcdir="$ZTST_testdir/$ZTST_srcdir"

# Set the function autoload paths to correspond to this build of zsh.
fpath=( $ZTST_srcdir/../Functions/*~*/CVS(/)
        $ZTST_srcdir/../Completion
        $ZTST_srcdir/../Completion/*/*~*/CVS(/) )

: ${TMPPREFIX:=/tmp/zsh}
# Temporary files for redirection inside tests.
ZTST_in=${TMPPREFIX}.ztst.in.$$
# hold the expected output
ZTST_out=${TMPPREFIX}.ztst.out.$$
ZTST_err=${TMPPREFIX}.ztst.err.$$
# hold the actual output from the test
ZTST_tout=${TMPPREFIX}.ztst.tout.$$
ZTST_terr=${TMPPREFIX}.ztst.terr.$$

ZTST_cleanup() {
  cd $ZTST_testdir
  rm -rf $ZTST_testdir/dummy.tmp $ZTST_testdir/*.tmp(N) \
    ${TMPPREFIX}.ztst*$$(N)
}

# This cleanup always gets performed, even if we abort.  Later,
# we should try and arrange that any test-specific cleanup
# always gets called as well.
trap - 'print cleaning up...
ZTST_cleanup' INT QUIT TERM
# Make sure it's clean now.
rm -rf dummy.tmp *.tmp

# Report failure.  Note that all output regarding the tests goes to stdout.
# That saves an unpleasant mixture of stdout and stderr to sort out.
ZTST_testfailed() {
  print -r "Test $ZTST_testname failed: $1"
  if [[ -n $ZTST_message ]]; then
    print -r "Was testing: $ZTST_message"
  fi
  print -r "$ZTST_testname: test failed."
  ZTST_testfailed=1
  return 1
}

# Print messages if $ZTST_verbose is non-empty
ZTST_verbose() {
  local lev=$1
  shift
  [[ -n $ZTST_verbose && $ZTST_verbose -ge $lev ]] && print -r -- $* >&8
}
ZTST_hashmark() {
  [[ ZTST_verbose -le 0 && -t 8 ]] && print -nu8 ${(pl:SECONDS::\#::\#\r:)}
  (( SECONDS > COLUMNS+1 && (SECONDS -= COLUMNS) ))
}

if [[ ! -r $ZTST_testname ]]; then
  ZTST_testfailed "can't read test file."
  exit 1
fi

exec 8>&1
exec 9<$ZTST_testname

# The current line read from the test file.
ZTST_curline=''
# The current section being run
ZTST_cursect=''

# Get a new input line.  Don't mangle spaces; set IFS locally to empty.
# We shall skip comments at this level.
ZTST_getline() {
  local IFS=
  while true; do
    read -r ZTST_curline <&9 || return 1
    [[ $ZTST_curline == \#* ]] || return 0
  done
}

# Get the name of the section.  It may already have been read into
# $curline, or we may have to skip some initial comments to find it.
# If argument present, it's OK to skip the reset of the current section,
# so no error if we find garbage.
ZTST_getsect() {
  local match mbegin mend

  while [[ $ZTST_curline != '%'(#b)([[:alnum:]]##)* ]]; do
    ZTST_getline || return 1
    [[ $ZTST_curline = [[:blank:]]# ]] && continue
    if [[ $# -eq 0 && $ZTST_curline != '%'[[:alnum:]]##* ]]; then
      ZTST_testfailed "bad line found before or after section:
$ZTST_curline"
      exit 1
    fi
  done
  # have the next line ready waiting
  ZTST_getline
  ZTST_cursect=${match[1]}
  ZTST_verbose 2 "ZTST_getsect: read section name: $ZTST_cursect"
  return 0
}

# Read in an indented code chunk for execution
ZTST_getchunk() {
  # Code chunks are always separated by blank lines or the
  # end of a section, so if we already have a piece of code,
  # we keep it.  Currently that shouldn't actually happen.
  ZTST_code=''
  # First find the chunk.
  while [[ $ZTST_curline = [[:blank:]]# ]]; do
    ZTST_getline || break
  done
  while [[ $ZTST_curline = [[:blank:]]##[^[:blank:]]* ]]; do
    ZTST_code="${ZTST_code:+${ZTST_code}
}${ZTST_curline}"
    ZTST_getline || break
  done
  ZTST_verbose 2 "ZTST_getchunk: read code chunk:
$ZTST_code"
  [[ -n $ZTST_code ]]
}

# Read in a piece for redirection.
ZTST_getredir() {
  local char=${ZTST_curline[1]} fn
  ZTST_redir=${ZTST_curline[2,-1]}
  while ZTST_getline; do
    [[ $ZTST_curline[1] = $char ]] || break
    ZTST_redir="${ZTST_redir}
${ZTST_curline[2,-1]}"
  done
  ZTST_verbose 2 "ZTST_getredir: read redir for '$char':
$ZTST_redir"

case $char in
  '<') fn=$ZTST_in
       ;;
  '>') fn=$ZTST_out
       ;;
  '?') fn=$ZTST_err
       ;;
   *)  ZTST_testfailed "bad redir operator: $char"
       return 1
       ;;
esac
if [[ $ZTST_flags = *q* ]]; then
  print -r -- "${(e)ZTST_redir}" >>$fn
else
  print -r -- "$ZTST_redir" >>$fn
fi

return 0
}

# Execute an indented chunk.  Redirections will already have
# been set up, but we need to handle the options.
ZTST_execchunk() {
  options=($ZTST_testopts)
  eval "$ZTST_code"
  ZTST_status=$?
  # careful... ksh_arrays may be in effect.
  ZTST_testopts=(${(kv)options[*]})
  options=(${ZTST_mainopts[*]})
  ZTST_verbose 2 "ZTST_execchunk: status $ZTST_status"
  return $ZTST_status
}

# Functions for preparation and cleaning.
# When cleaning up (non-zero string argument), we ignore status.
ZTST_prepclean() {
  # Execute indented code chunks.
  while ZTST_getchunk; do
    ZTST_execchunk >/dev/null || [[ -n $1 ]] ||
    ZTST_testfailed "non-zero status from preparation code:
$ZTST_code"
  done
}

# diff wrapper
ZTST_diff() {
  local diff_out diff_ret

  diff_out=$(diff "$@")
  diff_ret="$?"
  if [[ "$diff_ret" != "0" ]]; then
    echo "$diff_out"
  fi

  return "$diff_ret"
}
    
ZTST_test() {
  local last match mbegin mend found

  while true; do
    rm -f $ZTST_in $ZTST_out $ZTST_err
    touch $ZTST_in $ZTST_out $ZTST_err
    ZTST_message=''
    found=0

    ZTST_verbose 2 "ZTST_test: looking for new test"

    while true; do
      ZTST_verbose 2 "ZTST_test: examining line:
$ZTST_curline"
      case $ZTST_curline in
	%*) if [[ $found = 0 ]]; then
	      break 2
	    else
	      last=1
	      break
	    fi
	    ;;
	[[:space:]]#)
	    if [[ $found = 0 ]]; then
	      ZTST_getline || break 2
	      continue
	    else
	      break
	    fi
	    ;;
	[[:space:]]##[^[:space:]]*) ZTST_getchunk
	  if [[ $ZTST_curline == (#b)([-0-9]##)([[:alpha:]]#)(:*)# ]]; then
	    ZTST_xstatus=$match[1]
	    ZTST_flags=$match[2]
	    ZTST_message=${match[3]:+${match[3][2,-1]}}
	  else
	    ZTST_testfailed "expecting test status at:
$ZTST_curline"
	    return 1
	  fi
	  ZTST_getline
	  found=1
	  ;;
	'<'*) ZTST_getredir || return 1
	  found=1
	  ;;
	'>'*) ZTST_getredir || return 1
	  found=1
	  ;;
	'?'*) ZTST_getredir || return 1
	  found=1
	  ;;
	*) ZTST_testfailed "bad line in test block:
$ZTST_curline"
	  return 1
          ;;
      esac
    done

    # If we found some code to execute...
    if [[ -n $ZTST_code ]]; then
      ZTST_hashmark
      ZTST_verbose 1 "Running test: $ZTST_message"
      ZTST_verbose 2 "ZTST_test: expecting status: $ZTST_xstatus"
      ZTST_verbose 2 "Input: $ZTST_in, output: $ZTST_out, error: $ZTST_terr"

      ZTST_execchunk <$ZTST_in >$ZTST_tout 2>$ZTST_terr

      # First check we got the right status, if specified.
      if [[ $ZTST_xstatus != - && $ZTST_xstatus != $ZTST_status ]]; then
	ZTST_testfailed "bad status $ZTST_status, expected $ZTST_xstatus from:
$ZTST_code${$(<$ZTST_terr):+
Error output:
$(<$ZTST_terr)}"
	return 1
      fi

      ZTST_verbose 2 "ZTST_test: test produced standard output:
$(<$ZTST_tout)
ZTST_test: and standard error:
$(<$ZTST_terr)"

      # Now check output and error.
      if [[ $ZTST_flags != *d* ]] && ! ZTST_diff -c $ZTST_out $ZTST_tout; then
	ZTST_testfailed "output differs from expected as shown above for:
$ZTST_code${$(<$ZTST_terr):+
Error output:
$(<$ZTST_terr)}"
	return 1
      fi
      if [[ $ZTST_flags != *D* ]] && ! ZTST_diff -c $ZTST_err $ZTST_terr; then
	ZTST_testfailed "error output differs from expected as shown above for:
$ZTST_code"
	return 1
      fi
    fi
    ZTST_verbose 1 "Test successful."
    [[ -n $last ]] && break
  done

  ZTST_verbose 2 "ZTST_test: all tests successful"

  # reset message to keep ZTST_testfailed output correct
  ZTST_message=''
}


# Remember which sections we've done.
typeset -A ZTST_sects
ZTST_sects=(prep 0 test 0 clean 0)

print "$ZTST_testname: starting."

# Now go through all the different sections until the end.
ZTST_skipok=
while ZTST_getsect $ZTST_skipok; do
  case $ZTST_cursect in
    prep) if (( ${ZTST_sects[prep]} + ${ZTST_sects[test]} + \
	        ${ZTST_sects[clean]} )); then
	    ZTST_testfailed "\`prep' section must come first"
            exit 1
	  fi
	  ZTST_prepclean
	  ZTST_sects[prep]=1
	  ;;
    test)
	  if (( ${ZTST_sects[test]} + ${ZTST_sects[clean]} )); then
	    ZTST_testfailed "bad placement of \`test' section"
	    exit 1
	  fi
	  # careful here: we can't execute ZTST_test before || or &&
	  # because that affects the behaviour of traps in the tests.
	  ZTST_test
	  (( $? )) && ZTST_skipok=1
	  ZTST_sects[test]=1
	  ;;
    clean)
	   if (( ${ZTST_sects[test]} == 0 || ${ZTST_sects[clean]} )); then
	     ZTST_testfailed "bad use of \`clean' section"
	   else
	     ZTST_prepclean 1
	     ZTST_sects[clean]=1
	   fi
	   ZTST_skipok=
	   ;;
    *) ZTST_testfailed "bad section name: $ZTST_cursect"
       ;;
  esac
done

(( $ZTST_testfailed )) || print "$ZTST_testname: all tests successful."
ZTST_cleanup
exit $(( ZTST_testfailed ))

struct sudo_defs_types sudo_defs_table[] = {
    {
	"syslog_ifac", T_UINT,
	NULL
    }, {
	"syslog_igoodpri", T_UINT,
	NULL
    }, {
	"syslog_ibadpri", T_UINT,
	NULL
    }, {
	"syslog", T_LOGFAC|T_BOOL,
	"Syslog facility if syslog is being used for logging: %s"
    }, {
	"syslog_goodpri", T_LOGPRI,
	"Syslog priority to use when user authenticates successfully: %s"
    }, {
	"syslog_badpri", T_LOGPRI,
	"Syslog priority to use when user authenticates unsuccessfully: %s"
    }, {
	"long_otp_prompt", T_FLAG,
	"Put OTP prompt on its own line"
    }, {
	"ignore_dot", T_FLAG,
	"Ignore '.' in $PATH"
    }, {
	"mail_always", T_FLAG,
	"Always send mail when sudo is run"
    }, {
	"mail_badpass", T_FLAG,
	"Send mail if user authentication fails"
    }, {
	"mail_no_user", T_FLAG,
	"Send mail if the user is not in sudoers"
    }, {
	"mail_no_host", T_FLAG,
	"Send mail if the user is not in sudoers for this host"
    }, {
	"mail_no_perms", T_FLAG,
	"Send mail if the user is not allowed to run a command"
    }, {
	"tty_tickets", T_FLAG,
	"Use a separate timestamp for each user/tty combo"
    }, {
	"lecture", T_FLAG,
	"Lecture user the first time they run sudo"
    }, {
	"authenticate", T_FLAG,
	"Require users to authenticate by default"
    }, {
	"root_sudo", T_FLAG,
	"Root may run sudo"
    }, {
	"log_host", T_FLAG,
	"Log the hostname in the (non-syslog) log file"
    }, {
	"log_year", T_FLAG,
	"Log the year in the (non-syslog) log file"
    }, {
	"shell_noargs", T_FLAG,
	"If sudo is invoked with no arguments, start a shell"
    }, {
	"set_home", T_FLAG,
	"Set $HOME to the target user when starting a shell with -s"
    }, {
	"always_set_home", T_FLAG,
	"Always set $HOME to the target user's home directory"
    }, {
	"path_info", T_FLAG,
	"Allow some information gathering to give useful error messages"
    }, {
	"fqdn", T_FLAG,
	"Require fully-qualified hostnames in the sudoers file"
    }, {
	"insults", T_FLAG,
	"Insult the user when they enter an incorrect password"
    }, {
	"requiretty", T_FLAG,
	"Only allow the user to run sudo if they have a tty"
    }, {
	"env_editor", T_FLAG,
	"Visudo will honor the EDITOR environment variable"
    }, {
	"rootpw", T_FLAG,
	"Prompt for root's password, not the users's"
    }, {
	"runaspw", T_FLAG,
	"Prompt for the runas_default user's password, not the users's"
    }, {
	"targetpw", T_FLAG,
	"Prompt for the target user's password, not the users's"
    }, {
	"use_loginclass", T_FLAG,
	"Apply defaults in the target user's login class if there is one"
    }, {
	"set_logname", T_FLAG,
	"Set the LOGNAME and USER environment variables"
    }, {
	"stay_setuid", T_FLAG,
	"Only set the effective uid to the target user, not the real uid"
    }, {
	"env_reset", T_FLAG,
	"Reset the environment to a default set of variables"
    }, {
	"preserve_groups", T_FLAG,
	"Don't initialize the group vector to that of the target user"
    }, {
	"loglinelen", T_UINT|T_BOOL,
	"Length at which to wrap log file lines (0 for no wrap): %d"
    }, {
	"timestamp_timeout", T_INT|T_BOOL,
	"Authentication timestamp timeout: %d minutes"
    }, {
	"passwd_timeout", T_UINT|T_BOOL,
	"Password prompt timeout: %d minutes"
    }, {
	"passwd_tries", T_UINT,
	"Number of tries to enter a password: %d"
    }, {
	"umask", T_MODE|T_BOOL,
	"Umask to use or 0777 to use user's: 0%o"
    }, {
	"logfile", T_STR|T_BOOL|T_PATH,
	"Path to log file: %s"
    }, {
	"mailerpath", T_STR|T_BOOL|T_PATH,
	"Path to mail program: %s"
    }, {
	"mailerflags", T_STR|T_BOOL,
	"Flags for mail program: %s"
    }, {
	"mailto", T_STR|T_BOOL,
	"Address to send mail to: %s"
    }, {
	"mailsub", T_STR,
	"Subject line for mail messages: %s"
    }, {
	"badpass_message", T_STR,
	"Incorrect password message: %s"
    }, {
	"timestampdir", T_STR|T_PATH,
	"Path to authentication timestamp dir: %s"
    }, {
	"timestampowner", T_STR,
	"Owner of the authentication timestamp dir: %s"
    }, {
	"exempt_group", T_STR|T_BOOL,
	"Users in this group are exempt from password and PATH requirements: %s"
    }, {
	"passprompt", T_STR,
	"Default password prompt: %s"
    }, {
	"runas_default", T_STR,
	"Default user to run commands as: %s"
    }, {
	"editor", T_STR|T_PATH,
	"Path to the editor for use by visudo: %s"
    }, {
	"env_check", T_LIST|T_BOOL,
	"Environment variables to check for sanity:"
    }, {
	"env_delete", T_LIST|T_BOOL,
	"Environment variables to remove:"
    }, {
	"env_keep", T_LIST|T_BOOL,
	"Environment variables to preserve:"
    }, {
	"listpw_i", T_UINT,
	NULL
    }, {
	"verifypw_i", T_UINT,
	NULL
    }, {
	"listpw", T_PWFLAG,
	"When to require a password for 'list' pseudocommand: %s"
    }, {
	"verifypw", T_PWFLAG,
	"When to require a password for 'verify' pseudocommand: %s"
    }, {
	NULL, 0, NULL
    }
};

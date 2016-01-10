#include <security/pam_appl.h>
#include <security/pam_misc.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
#include <pwd.h>
#include <grp.h>
//#include <time.h>

const char* PAM_DOMAIN = "autox";

static struct pam_conv conv = {
  misc_conv,
  NULL
};

int pam_check_ret(pam_handle_t *handle, int ret, char msg[]);
int setup_pam(pam_handle_t *pamh, char user[]);

int main(int argc, char *argv[]) {
  int retval = 0;

/*struct tm *curtime;
  time_t *time;
  curtime = localtime(time);
  fprintf(stderr, "Start time = %i:%i on %i/%i/%i\n", curtime->tm_hour, curtime->tm_min, curtime->tm_mon, curtime->tm_mday, curtime->tm_year); */

  if (argc == 1) {
    printf("You mush supply a username!\n\te.g `autox myuser`\n");
    return (retval = 1);
  }

  if (argc > 2) {
    fprintf(stderr, "Too many arguments supplied!\n");
    return (retval = 1);
  }

  char *user = argv[1];
  pam_handle_t *pamh = NULL;
  pid_t cpid = 0;
  pid_t wpid = 0;
  cpid = fork();

  if (cpid == -1) {
    /* Could not start the X process. */
    fprintf(stderr, "Could not fork child process!\n");
    return 1;
  }

  /* Start the X session. */
  if (cpid == 0) {
    /* Setup the user's environment. */
    struct passwd *pw;
    pw = getpwnam(user);
    
    setenv("HOME", pw->pw_dir, 1);
    setenv("SHELL", pw->pw_shell, 1);
    setenv("USER", pw->pw_name, 1);
    setenv("LOGNAME", pw->pw_name, 1);
    /*setenv("DISPLAY", displayname, 1); */
    chdir(pw->pw_dir);

    /* Setup the user's groups list. */
    retval = initgroups(user, pw->pw_gid);
    if (retval == -1) {
      fprintf(stderr, "Could not set the user's groups!\n");
      switch (errno) {
        case EPERM:
          fprintf(stderr, "Insufficient priviliges to create group list!\n");
          break;
        case ENOMEM:
          fprintf(stderr, "Insufficient memory to create group list!\n");
          break;
      }
      return errno;
    }
    
    /* Check with PAM. */
    /* This must be done before the process uid and gid are changed. */
    retval = setup_pam(pamh, user);
    if (retval) {
      return retval;
    }

    /* Change the process' uid and gid. */
    setgid(pw->pw_gid);
    setuid(pw->pw_uid);
    
    /* All systems are a go! */
    system("xinit");
  }

  /* Wait for the X session to terminate so we can close the PAM session. */
  do {
    wpid = waitpid(cpid, &retval, WUNTRACED | WCONTINUED);
  } while (!WIFEXITED(retval) && !WIFSIGNALED(retval));

  retval = pam_close_session(pamh, 0);
  if (pam_check_ret(pamh, retval, "Close session error")) {
    return retval;
  }
  pam_end(pamh, retval);


  return retval;
}

int setup_pam(pam_handle_t *pamh, char user[]) {
  int retval = 0;

  /* Start PAM authentication. */
  retval = pam_start(PAM_DOMAIN, user, &conv, &pamh);
  if ( pam_check_ret(pamh, retval, "Open authentication error") ) {
    return retval;
  }

  /* Set the PAM user variable. */
  retval = pam_set_item(pamh, PAM_USER, user);
  if ( pam_check_ret(pamh, retval, "Set PAM user error") ) {
    return retval;
  }

  /* Establish the user's credentials. */
  retval = pam_setcred(pamh, PAM_ESTABLISH_CRED);
  if ( pam_check_ret(pamh, retval, "Establish credentials error") ) {
    return retval;
  }

  /* Make sure the user still has permissions to login. */
  retval = pam_acct_mgmt(pamh, 0);
  if ( pam_check_ret(pamh, retval, "Account management error") ) {
    return retval;
  }

  /* Start the session. */
  retval = pam_open_session(pamh, 0);
  if ( pam_check_ret(pamh, retval, "Open session error") ) {
    return retval;
  }

  return 0;
}

int pam_check_ret(pam_handle_t *handle, int ret, char msg[]) {
  switch (ret) {
    case PAM_ABORT:
    case PAM_ACCT_EXPIRED:
    case PAM_AUTH_ERR:
    case PAM_BUF_ERR:
    case PAM_CRED_ERR:
    case PAM_CRED_EXPIRED:
    case PAM_CRED_UNAVAIL:
    case PAM_PERM_DENIED:
    case PAM_SYSTEM_ERR:
    case PAM_USER_UNKNOWN:
      fprintf(stderr, "%s: %s\n", msg, pam_strerror(handle, ret));
      return ret;
  }

  return 0;
}

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <fcntl.h>
#include <unistd.h>
#include <security/pam_appl.h>
#include <security/pam_modules.h>
#include <security/pam_ext.h>

void	luksOpen(char *username, char *password, char *name, char *str)
{
  char	buff[1024] = "";

  if (str == NULL)
    {
      str = malloc(sizeof(char) * (22 + strlen(username)));
      str[0] = '\0';
      strcat(str, name);
      strcat(str, username);
      strcat(str, ".container\0");
    }
  strcat(buff, "echo ");
  strcat(buff, password);
  strcat(buff, " | cryptsetup luksOpen ");
  strcat(buff, str);
  strcat(buff, " ");
  strcat(buff, username);
  if (strcmp(name, "/mypam/pwd/") == 0)
    strcat(buff, "pwd");
  system(buff);
}

void	changePwd(char *pwd, char *password, char *username)
{
  char	buff[1024] = "";

  memset(buff, 0, 1024);
  strcat(buff, "printf '");
  strcat(buff, password);
  strcat(buff, "\\n");
  strcat(buff, pwd);
  strcat(buff, "\\n");
  strcat(buff, pwd);
  strcat(buff, "' | sudo cryptsetup luksAddKey /mypam/pwd/");
  strcat(buff, username);
  strcat(buff, ".container");
  system(buff);
  memset(buff, 0, 1024);
  strcat(buff, "printf '");
  strcat(buff, password);
  strcat(buff, "' | sudo cryptsetup luksRemoveKey /mypam/pwd/");
  strcat(buff, username);
  strcat(buff, ".container");
  system(buff);
}

char	*createPwd(char *username)
{
  char	*pwd = NULL;
  char	buff[1024] = "";
  char	buff2[1024] = "";
  int	fd;

  srand(time(NULL));
  pwd = malloc(sizeof(char) * 256);
  for (int i = 0; i < 255; i++)
    {
      pwd[i] = rand() % 57 + 65;
      while (pwd[i] >= 91 && pwd[i] <= 96)
	pwd[i] = rand() % 57 + 65;
    }
  pwd[255] = '\0';
  strcat(buff, "/home/");
  strcat(buff, username);
  strcat(buff, "/.pwdContainer/pwd");
  fd = open(buff, O_CREAT | O_TRUNC | O_RDWR, 0600);
  write(fd, pwd, 255);
  close(fd);
  memset(buff, 0, sizeof(buff));
  strcat(buff, "chown ");
  strcat(buff, username);
  strcat(buff, " ");
  strcat(buff, " /home/");
  strcat(buff, username);
  strcat(buff, "/.pwdContainer/pwd");
  system(buff);
  return (pwd);
}

void	closeContainer(char *username, char *name)
{
  char	buff[1024] = "";
  int fd;

  strcat(buff, "umount /home/");
  strcat(buff, username);
  strcat(buff, name);
  system(buff);
  memset(buff, 0, sizeof(buff));
  strcat(buff, "cryptsetup luksClose ");
  strcat(buff, username);
  if (strcmp(name, "/.pwdContainer/") == 0)
    strcat(buff, "pwd");
  system(buff);
  memset(buff, 0, sizeof(buff));
  strcat(buff, "rm -rf /home/");
  strcat(buff, username);
  strcat(buff, name);
  system(buff);
}

void	openContainer(char *username, char *name, char *container)
{
  char	buff[1024] = "";

  strcat(buff, "mkdir -p /home/");
  strcat(buff, username);
  strcat(buff, container);
  system(buff);
  memset(buff, 0, sizeof(buff));
  strcat(buff, "mount -t ext3 /dev/mapper/");
  strcat(buff, username);
  if (strcmp(name, "/mypam/pwd/") == 0)
    strcat(buff, "pwd");
  strcat(buff, " /home/");
  strcat(buff, username);
  strcat(buff, container);
  system(buff);
  memset(buff, 0, sizeof(buff));
  strcat(buff, "chown ");
  strcat(buff, username);
  strcat(buff, " /home/");
  strcat(buff, username);
  strcat(buff, container);
  system(buff);
  memset(buff, 0, sizeof(buff));
  strcat(buff, "chmod 700 /home/");
  strcat(buff, username);
  strcat(buff, container);
  system(buff);
}

char 	*createContainer(char *username, char *password, char *name, int i)
{
  char	*str;
  char	*strpwd;
  char	*pwd = NULL;
  char	buff[1024] = "";

  system("mkdir -p /mypam/pwd");
  if ((str = malloc(sizeof(char) * (22 + strlen(username)))) == NULL)
    return (NULL);
  str[0] = '\0';
  strcat(str, name);
  strcat(str, username);
  strcat(str, ".container");
  if (i == 0)
    {
      strcat(buff, "fallocate -l 10MB ");
      strcat(buff, str);
      system(buff);
    }
  else
    {
      strcat(buff, "fallocate -l 256MB ");
      strcat(buff, str);
      system(buff);
    }
  memset(buff, 0, sizeof(buff));
  strcat(buff, "chown ");
  strcat(buff, username);
  strcat(buff, " ");
  strcat(buff, str);
  system(buff);
  memset(buff, 0, sizeof(buff));
  strcat(buff, "chmod 700 ");
  strcat(buff, str);
  system(buff);
  memset(buff, 0, sizeof(buff));
  strcat(buff, "echo \"");
  strcat(buff, password);
  strcat(buff, "\" | cryptsetup luksFormat -c aes -h sha256 ");
  strcat(buff, str);
  system(buff);
  luksOpen(username, password, name, str);
  memset(buff, 0, sizeof(buff));
  strcat(buff, "mkfs.ext3 /dev/mapper/");
  strcat(buff, username);
  if (strcmp(name, "/mypam/pwd/") == 0)
    strcat(buff, "pwd");
  system(buff);
  if (i == 0)
    {
      openContainer(username, name, "/.pwdContainer/");
      pwd = createPwd(username);
      closeContainer(username, "/.pwdContainer/");
    }
  return (pwd);
}

char	*getPassword(char *username, char *password)
{
  char	*pwd;
  char	buff[1024] = "";

  luksOpen(username, password, "/mypam/pwd/", NULL);
  openContainer(username, "/mypam/pwd/", "/.pwdContainer/");
  strcat(buff, "/home/");
  strcat(buff, username);
  strcat(buff, "/.pwdContainer/pwd");
  int fd = open(buff, O_RDONLY);
  if (fd != -1)
    {
      if ((pwd = malloc(sizeof(char) * 256)) == NULL)
	return (NULL);
      read(fd, pwd, 255);
      pwd[255] = '\0';
    }
  else
    pwd = createPwd(username);
  close(fd);
  closeContainer(username, "/.pwdContainer/");
  return (pwd);
}

PAM_EXTERN int pam_sm_authenticate(pam_handle_t *pamh, int flags, int argc, const char **argv)
{
  int retval;
  char *password;
  const char *user;
  char *username;
  char	*pwd;
  char	*str;

  retval = pam_get_user(pamh, &user, "Username: ");
  username = strdup(user);
  pam_get_authtok(pamh, PAM_AUTHTOK, (const char**)&password, NULL);
  str = malloc(sizeof(char) * 17 + strlen(username));
  str[0] = '\0';
  strcat(str, "/mypam/");
  strcat(str, username);
  strcat(str, ".container");
  if (access(str, F_OK) == -1)
    {
      if ((pwd = createContainer(username, password, "/mypam/pwd/", 0)) == NULL)
	return PAM_AUTH_ERR;
      if (createContainer(username, pwd, "/mypam/", 1) == NULL)
	return PAM_AUTH_ERR;
    }
  else
    {
      if ((pwd = getPassword(username, password)) == NULL)
	return PAM_AUTH_ERR;
      luksOpen(username, pwd, "/mypam/", NULL);
    }
  openContainer(username, "/mypam/", "/secure_data-rw/");
  return PAM_AUTH_ERR;
}

PAM_EXTERN int pam_sm_close_session( pam_handle_t *pamh, int flags,int argc, const char **argv ) {
  const char *username;

  pam_get_user(pamh, &username, "Username: ");
  closeContainer(strdup(username), "/secure_data-rw/");
}

PAM_EXTERN int pam_sm_chauthtok(pam_handle_t *pamh, int flags, int argc, const char **argv) {
  static int	i = 0;
  static char	*password;
  char		*pwd;
  const char	*user;
  char		*username;

  if (i == 0)
    {
      pam_get_authtok(pamh, PAM_OLDAUTHTOK, (const char**)&password, NULL);
    }
  if (i == 1)
    {
      pam_get_authtok(pamh, PAM_AUTHTOK, (const char**)&pwd, NULL);
      pam_get_user(pamh, &user, "Username: ");
      username = strdup(user);
      changePwd(pwd, password, username);
    }
  i++;
  return PAM_SUCCESS;
}

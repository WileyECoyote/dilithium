

#ifndef NGROUPS_MAX
 #define NGROUPS_MAX 32
#endif

class Privileges
{

public:

  gid_t gid;
  uid_t uid;

  struct passwd *userinfo;

  //Privileges(passwd *userdata);
  Privileges();

  void initialize (passwd *userdata);
  bool drop_privileges();
  void restore_privileges(void);

protected:

  int   orig_ngroups;
  gid_t orig_groups[NGROUPS_MAX];

  gid_t orig_gid;
  uid_t orig_uid;

};

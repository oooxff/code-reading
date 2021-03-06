// test_open.cpp,v 1.9 2001/05/03 03:57:17 bala Exp

#include "ace/Naming_Context.h"
#include "ace/Log_Msg.h"

ACE_RCSID(Naming, test_open, "test_open.cpp,v 1.9 2001/05/03 03:57:17 bala Exp")

int
main (int argc, char **argv)
{
  const char *host = argc > 1 ? argv[1] : "-hlocalhost";
  const char *port = argc > 2 ? argv[2] : "-p20012";

  ACE_STATIC_SVC_REGISTER(ACE_Naming_Context);

  ACE_Naming_Context ns;
  ACE_Name_Options *name_options = ns.name_options ();

  const char *m_argv[] =
  {
    "MyName",
    "-cNET_LOCAL",
    host,
    port,
    NULL
  };
  int m_argc =
    sizeof (m_argv) / sizeof (char *) -1;

  name_options->parse_args (m_argc,
                            (char **) m_argv);

  int result = ns.open (ACE_Naming_Context::NET_LOCAL);
  ACE_DEBUG ((LM_DEBUG,
              "ACE_Naming_Context::open returned %d\n",
              result));
  if (result != 0)
    return result;
  else
    {
      char key[128];
      char val[32];
      char type[2];

      type[0] = '-';
      type[1] = '\0';

      int i = 0;

      for (int l = 1; l <= 1000 ; l++)
        {
          ACE_OS::sprintf (key,
                           "K_%05d_%05d",
                           (int) ACE_OS::getpid (),
                           l);
          ACE_OS::sprintf (val,
                           "Val%05d",
                           l);
          i = ns.bind (key,
                       val,
                       type);
          ACE_DEBUG ((LM_DEBUG,
                      "%d: bind of %s: %d\n",
                      ACE_OS::getpid (),
                      key,
                      i));

          if (i != 0)
            return -1;

        }

      result = ns.close ();
      ACE_DEBUG ((LM_DEBUG,
                  "ACE_Naming_Context::close returned %d\n",
                  result));
    }

  return result;
}

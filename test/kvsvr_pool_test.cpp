#include "../kvsvr_pool.h"
#include <parameters.h>
#include <TcpEpollServer.h>

using namespace kvsvr;

int main(int argc, char **argv)
{
  parameters::Parameters parameters(argc, argv);
  parameters.displayConfig();
  KVSVRPool pool(&parameters);
  pool.start();
  TcpEpollServer server(&pool, &parameters);
  server.handle_request();
}
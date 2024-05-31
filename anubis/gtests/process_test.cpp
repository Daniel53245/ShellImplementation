#include <gtest/gtest.h>
extern "C"{
        #include "../src/process_manager.h"
        #include "../src/tokeniser.h"
        #include "../src/utils.h"
        #include <string.h>
}

TEST(Path_resolv,test_path){
  path_list* test_list = create_path_list();
  test_list = append_path_to_list(test_list,"/bin");
  char *test_res = resolve_abs_path("ls",test_list);
  fprintf(stderr,"test_res:%s\n",test_res);
  EXPECT_EQ(0,strcmp(test_res,"/bin/ls"));
}
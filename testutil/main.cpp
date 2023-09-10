#include "stdio.h"

/*1. target lib */
#include "math.h"
#include "area.h"

/*2. util & test*/
#include "test.h"
#include "ja_list.h"
#include "ja_mem.h"

#include "ja_type.h"
#include "ja_crypto.h"

/* For ja list , Just test the function.*/
class Ja_list_test: public Test{
public:
  void test()
  {
    printf("ja list test 100 size, for ja_crypto_alg list, result : %d , time cost : %f nsec\n",
        test_ja_list(100),
        calculate_time_spend_nsec());
  }
  int test_ja_list(int count){

    JA_LIST_HEAD (alg_head);
    ja_list_head * pos;
    int i =0;
    ja_crypto_alg *alg = NULL;
    ja_crypto_alg *n = NULL;
    
    /*1.initial*/
    for(i = 0; i < count; i++){
      alg = (ja_crypto_alg *)ja_malloc(sizeof(ja_crypto_alg),1);
      if(!alg){
        return -1;
      }
      alg->cra_priority = i;
      ja_list_add_tail(&(alg->cra_list),&alg_head);
    }

    /*2.application*/
    ja_list_for_each(pos, &alg_head){
      printf("pos now is %x \n", pos);
    }

    ja_list_for_each_entry(alg, &alg_head, cra_list){
      printf("alg now is %x ,with cra_priority %d\n", alg, alg->cra_priority);
    }

    /*3. application for entry */
    ja_list_for_each_entry_safe(alg, n, &alg_head, cra_list){
      if((alg->cra_priority % 10 == 0) || (alg->cra_priority % 10 == 2)) {
        //printf("alg 1 now free %x ,with cra_priority %d\n", alg, alg->cra_priority);
        ja_list_del(&alg->cra_list);
        ja_free(alg);
      }
    }

    /*4. free resources */
    ja_list_for_each_entry_safe(alg, n, &alg_head, cra_list){
        //printf("alg 2 now free %x ,with cra_priority %d\n", alg, alg->cra_priority);
        ja_list_del_init(&alg->cra_list);
        ja_free(alg);
    }
    return 1;
  }
  int mathimatic_call(){
    return test_ja_list(100);
  }
};

int main()
{
  Ja_list_test ja_list;
  ja_list.test();
  return 0;
}

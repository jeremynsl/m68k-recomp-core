#include "m68k_step.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define CHECK(x) do{if(!(x)){fprintf(stderr,"line%d: %s\n",__LINE__,#x);exit(1);}}while(0)
/* Frozen prefix used by existing generated operation objects at7233852. */
typedef struct {M68kStepState state;M68kStepBus bus;void *user,*fault_escape;unsigned charge;} LegacyContext;
_Static_assert(offsetof(M68kStepContext,state)==offsetof(LegacyContext,state),"state prefix changed");
_Static_assert(offsetof(M68kStepContext,bus)==offsetof(LegacyContext,bus),"bus prefix changed");
_Static_assert(offsetof(M68kStepContext,user)==offsetof(LegacyContext,user),"user prefix changed");
_Static_assert(offsetof(M68kStepContext,fault_escape)==offsetof(LegacyContext,fault_escape),"fault prefix changed");
_Static_assert(offsetof(M68kStepContext,charge)==offsetof(LegacyContext,charge),"charge prefix changed");
typedef struct {uint8_t bytes[8192];uint32_t denied;unsigned ready,waits,fetches,queries,disabled;} Memory;
static M68kStepBusResult fetch(void *user,uint32_t address,uint16_t *word)
{
    Memory *m=user;++m->fetches;
    if(address==m->denied)return M68K_BUS_FAULT;
    if(address==0x600&&!m->ready)return M68K_BUS_WAIT;
    if(address>=sizeof m->bytes&&address<0xff0000u)return M68K_BUS_FAULT;
    unsigned offset=address&(sizeof m->bytes-1);
    *word=(uint16_t)(((uint32_t)m->bytes[offset]<<8)|m->bytes[(offset+1)&(sizeof m->bytes-1)]);return M68K_BUS_OK;
}
static int wait_memory(void *user,uint32_t address,int write)
{Memory *m=user;CHECK(address==0x600&&!write);++m->waits;m->ready=1;return 1;}
static const uint8_t *span(void *user,uint32_t address,size_t bytes)
{
    Memory *m=user;++m->queries;
    if(m->disabled || (address>=sizeof m->bytes&&address<0xff0000u) ||
       (address<=m->denied&&m->denied-address<bytes) ||
       (!m->ready&&address<=0x600&&0x600-address<bytes))return NULL;
    unsigned offset=address&(sizeof m->bytes-1);
    return bytes<=sizeof m->bytes-offset?m->bytes+offset:NULL;
}
static void operation(M68kStepContext *c)
{CHECK(!m68k_step_code_span(c,NULL));++c->state.regs.D[0];}
static void compare(Memory *source,M68kStepEntry *entry,int enabled,unsigned expected_queries)
{
    Memory a=*source,b=*source;M68kStepContext slow,fast;M68kStepBus bus={0};
    bus.fetch16=fetch;bus.wait=wait_memory;
    m68k_step_init(&slow,&bus,&a);m68k_step_init(&fast,&bus,&b);
    slow.state.regs.PC=fast.state.regs.PC=entry->pc;
    slow.state.regs.SR=fast.state.regs.SR=0x2700;
    CHECK(m68k_step_code_span(&fast,enabled?span:NULL));
    M68kStepResult old=m68k_step_execute(&slow,entry),actual=m68k_step_execute(&fast,entry);
    CHECK(old==actual&&!memcmp(&slow.state,&fast.state,sizeof slow.state));
    CHECK(!memcmp(a.bytes,b.bytes,sizeof a.bytes)&&a.waits==b.waits);
    CHECK(!slow.fault_escape&&!fast.fault_escape&&b.queries==expected_queries);
    if(!enabled||source->disabled||source->denied!=UINT32_MAX||!source->ready)CHECK(a.fetches==b.fetches);
    else if(expected_queries&&old==M68K_STEP_OK)CHECK(b.fetches==0&&a.fetches==entry->word_count);
}
int main(void)
{
    Memory m={0};m.denied=UINT32_MAX;m.ready=1;
    CHECK(!m68k_step_code_span(NULL,NULL));
    const uint32_t pcs[]={0x400,0xff0400,0xfffffe,0x401};
    for(unsigned p=0;p<sizeof pcs/sizeof *pcs;++p)for(unsigned n=1;n<=11;++n) {
        M68kStepEntry entry={0};entry.pc=pcs[p];entry.word_count=(uint8_t)n;entry.cycles=8;entry.execute=operation;
        for(unsigned i=0;i<n;++i) {
            entry.words[i]=(uint16_t)(0x3000u+i*17u+n);
            unsigned offset=(entry.pc+i*2u)&8191u;
            m.bytes[offset]=(uint8_t)(entry.words[i]>>8);m.bytes[(offset+1)&8191u]=(uint8_t)entry.words[i];
        }
        unsigned queries=!(entry.pc&1u)&&n*2u<=0x1000000u-entry.pc;
        compare(&m,&entry,0,0);compare(&m,&entry,1,queries);
        m.disabled=1;compare(&m,&entry,1,queries);m.disabled=0;
        m.bytes[(entry.pc+2u*n-1u)&8191u]^=0x80;compare(&m,&entry,1,queries); /* live extension/opcode */
    }
    M68kStepEntry entry={0x5fe,{0x303c,0x1234},2,8,operation};
    m.bytes[0x5fe]=0x30;m.bytes[0x5ff]=0x3c;m.bytes[0x600]=0x12;m.bytes[0x601]=0x34;
    m.ready=0;compare(&m,&entry,1,1); /* partial guard then physical WAIT */
    m.ready=1;m.denied=0x600;compare(&m,&entry,1,1); /* extension fetch fault */
    puts("owned code spans: all guard lengths/live bytes, odd/wrapped PCs, NULL fallback, WAIT/fault and legacy operation prefix PASS");
    return 0;
}

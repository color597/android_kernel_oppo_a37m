#include <linux/init.h>
#include <linux/irq.h>
#include <linux/interrupt.h>
#include <linux/irqreturn.h>
#include <linux/jiffies.h>
#include <linux/clockchips.h>
#include <linux/types.h>

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/spinlock.h>
#include <asm/arch_timer.h>
//#include <linux/smp.h>

#include <linux/io.h>

#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_irq.h>

#include <mach/mt_cpuxgpt.h>

#include <mach/mt_secure_api.h>

#if defined(CONFIG_ARM_PSCI) || defined(CONFIG_MTK_PSCI)
#define CPUXGPT_BASE 	cpuxgpt_regs_phy
#else
#define CPUXGPT_BASE 	cpuxgpt_regs
#endif

#define INDEX_BASE  	(CPUXGPT_BASE+0x0674)
#define CTL_BASE    	(CPUXGPT_BASE+0x0670)

#define CPUXGPT_IRQID_BASE cpuxgpt_irq[0]
//extern void mt_gic_cfg_irq2cpu(unsigned int irq, unsigned int cpu, unsigned int set);

static DEFINE_SPINLOCK(cpuxgpt_reg_lock);
static irqreturn_t(*user_handlers[CPUXGPTNUMBERS])(int irq, void *dev_id) = {0};
static unsigned int g_ctl = 0;

static void __iomem *cpuxgpt_regs;
phys_addr_t	cpuxgpt_regs_phy;

static int cpuxgpt_irq[CPUXGPTNUMBERS];

static irqreturn_t __cpuxgpt0_irq_handler(int irq, void *dev_id);
static irqreturn_t __cpuxgpt1_irq_handler(int irq, void *dev_id);
static irqreturn_t __cpuxgpt2_irq_handler(int irq, void *dev_id);
static irqreturn_t __cpuxgpt3_irq_handler(int irq, void *dev_id);
static irqreturn_t __cpuxgpt4_irq_handler(int irq, void *dev_id);
static irqreturn_t __cpuxgpt5_irq_handler(int irq, void *dev_id);
static irqreturn_t __cpuxgpt6_irq_handler(int irq, void *dev_id);
static irqreturn_t __cpuxgpt7_irq_handler(int irq, void *dev_id);

static const struct of_device_id cpuxgpt_addr_ids[] __initconst = {
	{.compatible = "mediatek,MCUCFG"},
	{},
};

static irqreturn_t(*cpuxgpt_irq_handler[])(int irq, void *dev_id) = {
    __cpuxgpt0_irq_handler,
    __cpuxgpt1_irq_handler,
    __cpuxgpt2_irq_handler,
    __cpuxgpt3_irq_handler,
    __cpuxgpt4_irq_handler,
    __cpuxgpt5_irq_handler,
    __cpuxgpt6_irq_handler,
    __cpuxgpt7_irq_handler,
};//support 8 timer call back


static unsigned int __read_cpuxgpt(unsigned int reg_index)
{
  	unsigned int value = 0;
  	
#if defined(CONFIG_ARM_PSCI) || defined(CONFIG_MTK_PSCI)
		mcusys_smc_write_phy(INDEX_BASE, reg_index);
#else		
		mcusys_smc_write(INDEX_BASE, reg_index);
#endif
 
 		//DRV_WriteReg32(INDEX_BASE,reg_index);
  	value = DRV_Reg32(cpuxgpt_regs+0x0670);
  	return value;
}

static void __write_cpuxgpt(unsigned int reg_index,unsigned int value )
{
	/* pr_debug("%s: ATF_SUPPORT\n", __func__); */
		
#if defined(CONFIG_ARM_PSCI) || defined(CONFIG_MTK_PSCI)
		mcusys_smc_write_phy(INDEX_BASE, reg_index);
		mcusys_smc_write_phy(CTL_BASE, value);	
#else
		mcusys_smc_write(INDEX_BASE, reg_index);
		mcusys_smc_write(CTL_BASE, value);
#endif		
}

static int __get_irq_id(int id)
{
		if(id < CPUXGPTNUMBERS)
			return cpuxgpt_irq[id];
			
	pr_debug("%s:fine irq id error\n", __func__);
		return -1;
}

static void  __cpuxgpt_enable(void)
{
   	unsigned int tmp = 0;
	spin_lock(&cpuxgpt_reg_lock);
   	tmp = __read_cpuxgpt(INDEX_CTL_REG);
   	tmp  |= EN_CPUXGPT;
   	__write_cpuxgpt(INDEX_CTL_REG,tmp);
	spin_unlock(&cpuxgpt_reg_lock);
}

static void  __cpuxgpt_disable(void)
{
   	unsigned int tmp = 0;
	spin_lock(&cpuxgpt_reg_lock);
   	tmp = __read_cpuxgpt(INDEX_CTL_REG);
   	tmp  &= (~EN_CPUXGPT);
   	__write_cpuxgpt(INDEX_CTL_REG,tmp);
	spin_unlock(&cpuxgpt_reg_lock);
}

static void  __cpuxgpt_halt_on_debug_en(int en)
{
   	unsigned int tmp = 0;
	spin_lock(&cpuxgpt_reg_lock);
   	tmp = __read_cpuxgpt(INDEX_CTL_REG);
	if(1 == en)
	{
   		tmp  |= EN_AHLT_DEBUG;
	}
	if(0==en)
	{
		tmp  &= (~EN_AHLT_DEBUG);
	}
   	__write_cpuxgpt(INDEX_CTL_REG,tmp);
	spin_unlock(&cpuxgpt_reg_lock);
}

static void __cpuxgpt_set_clk(unsigned int div)
{
	unsigned int tmp = 0;
   	
	/* pr_debug("%s fwq  div is  0x%x\n", __func__, div); */
	if( div!=CLK_DIV1 &&  div!=CLK_DIV2 && div!=CLK_DIV4)
	{
		pr_debug("%s error: div is not right\n", __func__);
	}
	spin_lock(&cpuxgpt_reg_lock);
   	tmp = __read_cpuxgpt(INDEX_CTL_REG);
  	tmp &= CLK_DIV_MASK;
  	tmp |= div;
  	__write_cpuxgpt(INDEX_CTL_REG,tmp);
	spin_unlock(&cpuxgpt_reg_lock);
}

static void __cpuxgpt_set_init_cnt(unsigned int countH,unsigned int  countL)
{
		spin_lock(&cpuxgpt_reg_lock);
   	__write_cpuxgpt(INDEX_CNT_H_INIT,countH);
   	__write_cpuxgpt(INDEX_CNT_L_INIT,countL); // update count when countL programmed
   	spin_unlock(&cpuxgpt_reg_lock);
}

static unsigned int __cpuxgpt_irq_en(int cpuxgpt_num)
{
  unsigned int tmp = 0;
  
	spin_lock(&cpuxgpt_reg_lock);
  tmp = __read_cpuxgpt(INDEX_IRQ_MASK);
	tmp |=(1<<cpuxgpt_num);
  __write_cpuxgpt(INDEX_IRQ_MASK,tmp);
	spin_unlock(&cpuxgpt_reg_lock);
	
	return 0;
}

static unsigned int __cpuxgpt_irq_dis(int cpuxgpt_num)
{
    unsigned int tmp = 0;
	spin_lock(&cpuxgpt_reg_lock);
    tmp = __read_cpuxgpt(INDEX_IRQ_MASK);
	tmp &=(~(1<<cpuxgpt_num));
    __write_cpuxgpt(INDEX_IRQ_MASK,tmp);
	spin_unlock(&cpuxgpt_reg_lock);

	return 0;
}

static unsigned int __cpuxgpt_set_cmp(CPUXGPT_NUM cpuxgpt_num, int countH,int countL)
{
	spin_lock(&cpuxgpt_reg_lock);
   	__write_cpuxgpt(INDEX_CMP_BASE+(cpuxgpt_num*0x8)+0x4,countH); 
   	__write_cpuxgpt(INDEX_CMP_BASE+(cpuxgpt_num*0x8),countL);// update CMP  when countL programmed
	spin_unlock(&cpuxgpt_reg_lock);
	return 0;
}

static irqreturn_t __cpuxgpt0_irq_handler(int irq, void *dev_id)
{
	__cpuxgpt_irq_dis(0);
	user_handlers[0](irq,dev_id);
	return IRQ_HANDLED;
}
static irqreturn_t __cpuxgpt1_irq_handler(int irq, void *dev_id)
{
	__cpuxgpt_irq_dis(1);
	user_handlers[1](irq,dev_id);
	return IRQ_HANDLED;
}
static irqreturn_t __cpuxgpt2_irq_handler(int irq, void *dev_id)
{
	__cpuxgpt_irq_dis(2);
	user_handlers[2](irq,dev_id);
	return IRQ_HANDLED;
}
static irqreturn_t __cpuxgpt3_irq_handler(int irq, void *dev_id)
{
	__cpuxgpt_irq_dis(3);
	user_handlers[3](irq,dev_id);
	return IRQ_HANDLED;
}
static irqreturn_t __cpuxgpt4_irq_handler(int irq, void *dev_id)
{
	__cpuxgpt_irq_dis(4);
	user_handlers[4](irq,dev_id);
	return IRQ_HANDLED;
}
static irqreturn_t __cpuxgpt5_irq_handler(int irq, void *dev_id)
{
	__cpuxgpt_irq_dis(5);
	user_handlers[5](irq,dev_id);
	return IRQ_HANDLED;
}
static irqreturn_t __cpuxgpt6_irq_handler(int irq, void *dev_id)
{
	__cpuxgpt_irq_dis(6);
	user_handlers[6](irq,dev_id);
	return IRQ_HANDLED;
}

static irqreturn_t __cpuxgpt7_irq_handler(int irq, void *dev_id)
{
	__cpuxgpt_irq_dis(7);
	user_handlers[7](irq,dev_id);
	return IRQ_HANDLED;
}

int cpu_xgpt_set_cmp_HL(CPUXGPT_NUM cpuxgpt_num, int countH,int countL)
{
	__cpuxgpt_set_cmp(cpuxgpt_num,countH, countL);
	__cpuxgpt_irq_en(cpuxgpt_num);

	return 0;
}

static void __init mt_cpuxgpt_init(struct device_node *node)
{
    int i;
    struct device_node *config_node;
    struct resource	phy_addr;
	
		//freq=SYS_CLK_RATE
//		if (of_property_read_u32(node, "clock-frequency", &freq))
//			pr_err("clock-frequency not set in the .dts file");	
					
		/* Setup IRQ numbers */
		for (i = CPUXGPT0; i < CPUXGPTNUMBERS; i++)
			cpuxgpt_irq[i] = irq_of_parse_and_map(node, i);
		
		/* Setup IO addresses based on MCUCFG */
		config_node = of_find_matching_node(NULL, cpuxgpt_addr_ids);
		if (!config_node)
			pr_err("No timer");
		cpuxgpt_regs = of_iomap(config_node, 0);

		of_address_to_resource(config_node, 0, &phy_addr);
		cpuxgpt_regs_phy = phy_addr.start;

//		pr_alert("mt_cpuxgpt_init: cpuxgpt_regs=0x%x, irq0=%d, irq1=%d, irq2=%d, irq3=%d, irq4=%d, irq5=%d, irq6=%d, irq7=%d \n",
//			 cpuxgpt_regs, cpuxgpt_irq[0], cpuxgpt_irq[1], cpuxgpt_irq[2], cpuxgpt_irq[3], cpuxgpt_irq[4], cpuxgpt_irq[5],
//			 cpuxgpt_irq[6], cpuxgpt_irq[7]);	
	
}


/**********************	export area *********************/
u64 localtimer_get_phy_count(void)
{
	u64 cval = 0;

	//cval = arch_counter_get_cntvct();
	cval = arch_counter_get_cntpct();

	return cval;
}
EXPORT_SYMBOL(localtimer_get_phy_count);

int cpu_xgpt_register_timer(unsigned int id,irqreturn_t (*func)(int irq, void *dev_id))
{	
    int ret =0;
		int irq_id=0;
		char* name;
		
		if(id>7 || id<0)
		{
		pr_debug("%s: err idnumber id=%d should be 0~7\n", __func__, id);
				return -1;
		}
		
		if(!cpu_present(id))
		{
			pr_debug("%s: err idnumber id=%d not present\n", __func__, id);
			return -1;
		}

		switch(id)
		{
			case 0:
				name = "mtk_cpuxgpt0";
				break;
			case 1: 
				name = "mtk_cpuxgpt1";
				break;
			case 2:
				name = "mtk_cpuxgpt2";
				break;
			case 3:
				name = "mtk_cpuxgpt3";
				break;
			case 4:
				name = "mtk_cpuxgpt4";
				break;
			case 5:
				name = "mtk_cpuxgpt5";
				break;
			case 6:
				name = "mtk_cpuxgpt6";
				break;
			case 7:
				name = "mtk_cpuxgpt7";
				break;
				
		}
		if (func) 
		{
		  	user_handlers[id] = func;
		}
		
		//sprintf(name, "mtk_cpuxgpt%d", id);
		
		irq_id  = __get_irq_id(id);
		
		/*cpuxgpt assigne  for per core*/
#ifdef CONFIG_MTK_GIC
		irq_force_affinity(irq_id, cpumask_of(id));
		
#else		
		mt_gic_cfg_irq2cpu(irq_id,0,0);/*don't trigger IRQ to CPU0*/
		mt_gic_cfg_irq2cpu(irq_id,(irq_id - CPUXGPT_IRQID_BASE)%num_possible_cpus(),1); /*trigger IRQ to CPUx*/
#endif		
		
		ret = request_irq(irq_id, (irq_handler_t)cpuxgpt_irq_handler[id], IRQF_TRIGGER_HIGH | IRQF_PERCPU, name, NULL);
		if(ret != 0)
		{
		pr_debug("%s:%s fail to register irq\n", __func__, name);
			return ret;
		}
		
	pr_debug("%s:%s register irq (%d) ok\n", __func__, name , irq_id);
		
		return 0; 	
}
EXPORT_SYMBOL(cpu_xgpt_register_timer);

int cpu_xgpt_set_timer(int id,u64 ns)
{
	u64 count=0;
	u64 now=0;
	u64 set_count=0;
	unsigned int set_count_lo = 0;
  unsigned int set_count_hi = 0;
	count=ns;
	now =localtimer_get_phy_count();
	do_div(count,1000/13);

	set_count = count + now;
	set_count_lo = 0x00000000FFFFFFFF & set_count;
	set_count_hi = (0xFFFFFFFF00000000 & set_count)>>32;

	pr_debug("%s:set cpuxgpt(%d) count(%u,%u)\n", __func__, id, set_count_hi, set_count_lo);
	
	__cpuxgpt_set_cmp(id,set_count_hi, set_count_lo);
	__cpuxgpt_irq_en(id);
   	return 0;	
}
EXPORT_SYMBOL(cpu_xgpt_set_timer);

void enable_cpuxgpt(void)
{
   __cpuxgpt_enable();
	pr_debug("%s: reg(%x)\n", __func__, __read_cpuxgpt(INDEX_CTL_REG));
}
EXPORT_SYMBOL(enable_cpuxgpt);

void disable_cpuxgpt(void)
{
   __cpuxgpt_disable();
	pr_debug("%s: reg(%x)\n", __func__, __read_cpuxgpt(INDEX_CTL_REG));
}
EXPORT_SYMBOL(disable_cpuxgpt);

void set_cpuxgpt_clk(unsigned int div)
{
   __cpuxgpt_set_clk(div);
	pr_debug("%s: reg(%x)\n", __func__, __read_cpuxgpt(INDEX_CTL_REG));
}
EXPORT_SYMBOL(set_cpuxgpt_clk);

void restore_cpuxgpt(void)
{
	__write_cpuxgpt(INDEX_CTL_REG,g_ctl);
	pr_debug("g_ctl:0x%x, %s\n", __read_cpuxgpt(INDEX_CTL_REG), __func__);
	
}
EXPORT_SYMBOL(restore_cpuxgpt);

void save_cpuxgpt(void)
{
	g_ctl = __read_cpuxgpt(INDEX_CTL_REG);
	pr_debug("g_ctl:0x%x, %s\n", g_ctl, __func__);
}
EXPORT_SYMBOL(save_cpuxgpt);

unsigned int cpu_xgpt_irq_dis(int cpuxgpt_num)
{
	__cpuxgpt_irq_dis(cpuxgpt_num);

	return 0;
}
EXPORT_SYMBOL(cpu_xgpt_irq_dis);

int cpu_xgpt_set_cmp(CPUXGPT_NUM cpuxgpt_num,u64 count)
{

	unsigned int set_count_lo = 0;
    unsigned int set_count_hi = 0;
	
	set_count_lo = 0x00000000FFFFFFFF & count;
	set_count_hi = (0xFFFFFFFF00000000 & count)>>32;
	
	cpu_xgpt_set_cmp_HL(cpuxgpt_num,set_count_hi,set_count_lo);
	return 0;
}
EXPORT_SYMBOL(cpu_xgpt_set_cmp);

void cpu_xgpt_set_init_count(unsigned int countH,unsigned int  countL)
{
	 __cpuxgpt_set_init_cnt(countH,countL);
	 
}
EXPORT_SYMBOL(cpu_xgpt_set_init_count);

void cpu_xgpt_halt_on_debug_en(int en)
{
	__cpuxgpt_halt_on_debug_en(en); 
}
EXPORT_SYMBOL(cpu_xgpt_halt_on_debug_en);

void generic_timer_backup(void)
{
		u64 cval;
		
		cval = arch_counter_get_cntpct();
		__cpuxgpt_set_init_cnt((__u32)(cval >> 32), (__u32)(cval & 0xffffffff));
}
EXPORT_SYMBOL(generic_timer_backup);

CLOCKSOURCE_OF_DECLARE(mtk_cpuxgpt, "mediatek,cpuxgpt", mt_cpuxgpt_init);

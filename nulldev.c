#include <linux/module.h>
#include <linux/moduleparam.h>
#include <scsi/scsi_host.h>

//#include "ufs/ufshcd.h"
#include "/root/linufs/drivers/scsi/ufs/ufshcd.h"

#ifdef CONFIG_SCSI

struct ufs_hba *null_hba;

static int null_ufshcd_queuecommand(struct Scsi_Host *host, struct scsi_cmnd *cmd)
{
	pr_info("NULL_DEV: null_ufshcd_queuecommand(), started\n");
	pr_info("NULL_DEV: null_ufshcd_queuecommand(), completed\n");
	return 0;
}

static int null_ufshcd_slave_alloc(struct scsi_device *sdev)
{
	pr_info("NULL_DEV: null_ufshcd_slave_alloc(), started\n");
	pr_info("NULL_DEV: null_ufshcd_slave_alloc(), completed\n");
	return 0;
}

static int null_ufshcd_slave_configure(struct scsi_device *sdev)
{
	pr_info("NULL_DEV: null_ufshcd_slave_configure(), started\n");
	pr_info("NULL_DEV: null_ufshcd_slave_configure(), completed\n");
	return 0;
}

static void null_ufshcd_slave_destroy(struct scsi_device *sdev)
{
	pr_info("NULL_DEV: null_ufshcd_slave_destroy(), started\n");
	pr_info("NULL_DEV: null_ufshcd_slave_destroy(), completed\n");
}

static int null_ufshcd_change_queue_depth(struct scsi_device *sdev, int depth)
{
	pr_info("NULL_DEV: null_ufshcd_change_queue_depth(), started\n");
	pr_info("NULL_DEV: null_ufshcd_change_queue_depth(), completed\n");
	return 0;
}

static int null_ufshcd_abort(struct scsi_cmnd *cmd)
{
	pr_info("NULL_DEV: null_ufshcd_abort(), started\n");
	pr_info("NULL_DEV: null_ufshcd_abort(), completed\n");
	return 0;
}

static int null_ufshcd_eh_device_reset_handler(struct scsi_cmnd *cmd)
{
	pr_info("NULL_DEV: null_ufshcd_eh_device_reset_handler(), started\n");
	pr_info("NULL_DEV: null_ufshcd_eh_device_reset_handler(), completed\n");
	return 0;
}

static int null_ufshcd_eh_host_reset_handler(struct scsi_cmnd *cmd)
{
	pr_info("NULL_DEV: null_ufshcd_eh_host_reset_handler(), started\n");
	pr_info("NULL_DEV: null_ufshcd_eh_host_reset_handler(), completed\n");
	return 0;
}

static enum blk_eh_timer_return null_ufshcd_eh_timed_out(struct scsi_cmnd *scmd)
{
	pr_info("NULL_DEV: null_ufshcd_eh_timed_out(), started\n");
	pr_info("NULL_DEV: null_ufshcd_eh_timed_out(), completed\n");
	return BLK_EH_NOT_HANDLED;
}

static struct scsi_host_template null_ufshcd_driver_template = {
	.module			= THIS_MODULE,
	.name			= UFSHCD,
	.proc_name		= UFSHCD,
	.queuecommand		= null_ufshcd_queuecommand,
	.slave_alloc		= null_ufshcd_slave_alloc,
	.slave_configure	= null_ufshcd_slave_configure,
	.slave_destroy		= null_ufshcd_slave_destroy,
	.change_queue_depth	= null_ufshcd_change_queue_depth,
	.eh_abort_handler	= null_ufshcd_abort,
	.eh_device_reset_handler = null_ufshcd_eh_device_reset_handler,
	.eh_host_reset_handler   = null_ufshcd_eh_host_reset_handler,
	.eh_timed_out		= null_ufshcd_eh_timed_out,
	.this_id		= -1,
	.sg_tablesize		= 128,
	.cmd_per_lun		= 32,
	.can_queue		= 32,
	.max_host_blocked	= 1,
	.track_queue_depth	= 1,
};

static int null_ufshcd_memory_alloc(struct ufs_hba *hba)
{
	size_t utmrdl_size, utrdl_size, ucdl_size;

	pr_info("NULL_DEV: null_ufshcd_memory_alloc(), started\n");
	
	/* Allocate memory for UTP command descriptors */
	ucdl_size = (sizeof(struct utp_transfer_cmd_desc) * hba->nutrs);
	hba->ucdl_base_addr = dmam_alloc_coherent(hba->dev, ucdl_size, &hba->ucdl_dma_addr, GFP_KERNEL);
	if (!hba->ucdl_base_addr || WARN_ON(hba->ucdl_dma_addr & (PAGE_SIZE - 1))) {
		pr_info("NULL_DEV: null_ufshcd_memory_alloc(), command descriptor memory allocation failed\n");
		goto out;
	}

	/* Allocate memory for UTP Transfer descriptors UFSHCI requires 1024 byte alignment of UTRD */
	utrdl_size = (sizeof(struct utp_transfer_req_desc) * hba->nutrs);
	hba->utrdl_base_addr = dmam_alloc_coherent(hba->dev, utrdl_size, &hba->utrdl_dma_addr, GFP_KERNEL);
	if (!hba->utrdl_base_addr || WARN_ON(hba->utrdl_dma_addr & (PAGE_SIZE - 1))) {
		pr_info("NULL_DEV: null_ufshcd_memory_alloc(), transfer descriptor memory allocation failed\n");
		goto out;
	}

	/* Allocate memory for UTP Task Management descriptors UFSHCI requires 1024 byte alignment of UTMRD */
	utmrdl_size = sizeof(struct utp_task_req_desc) * hba->nutmrs;
	hba->utmrdl_base_addr = dmam_alloc_coherent(hba->dev, utmrdl_size, &hba->utmrdl_dma_addr, GFP_KERNEL);
	if (!hba->utmrdl_base_addr || WARN_ON(hba->utmrdl_dma_addr & (PAGE_SIZE - 1))) {
		pr_info("NULL_DEV: null_ufshcd_memory_alloc(), task management descriptor memory allocation failed\n");
		goto out;
	}

	/* Allocate memory for local reference block */
	hba->lrb = devm_kzalloc(hba->dev, hba->nutrs * sizeof(struct ufshcd_lrb), GFP_KERNEL);
	if (!hba->lrb) {
		pr_info("NULL_DEV: null_ufshcd_memory_alloc(), LRB memory allocation failed\n");
		goto out;
	}
	pr_info("NULL_DEV: null_ufshcd_memory_alloc(), completed\n");
	return 0;
	
out:
	pr_info("NULL_DEV: null_ufshcd_memory_alloc(), failed\n");
	return -ENOMEM;
}

static void null_ufshcd_host_memory_configure(struct ufs_hba *hba)
{
	struct utp_transfer_cmd_desc *cmd_descp;
	struct utp_transfer_req_desc *utrdlp;
	dma_addr_t cmd_desc_dma_addr;
	dma_addr_t cmd_desc_element_addr;
	u16 response_offset;
	u16 prdt_offset;
	int cmd_desc_size;
	int i;

	pr_info("NULL_DEV: null_ufshcd_host_memory_configure(), started\n");
	utrdlp = hba->utrdl_base_addr;
	cmd_descp = hba->ucdl_base_addr;
	response_offset = offsetof(struct utp_transfer_cmd_desc, response_upiu);
	prdt_offset = offsetof(struct utp_transfer_cmd_desc, prd_table);
	cmd_desc_size = sizeof(struct utp_transfer_cmd_desc);
	cmd_desc_dma_addr = hba->ucdl_dma_addr;

	for (i = 0; i < hba->nutrs; i++) {
		/* Configure UTRD with command descriptor base address */
		cmd_desc_element_addr = (cmd_desc_dma_addr + (cmd_desc_size * i));
		utrdlp[i].command_desc_base_addr_lo = cpu_to_le32(lower_32_bits(cmd_desc_element_addr));
		utrdlp[i].command_desc_base_addr_hi = cpu_to_le32(upper_32_bits(cmd_desc_element_addr));

		/* Response upiu and prdt offset should be in double words */
		if (hba->quirks & UFSHCD_QUIRK_PRDT_BYTE_GRAN) {
			utrdlp[i].response_upiu_offset = cpu_to_le16(response_offset);
			utrdlp[i].prd_table_offset = cpu_to_le16(prdt_offset);
			utrdlp[i].response_upiu_length = cpu_to_le16(ALIGNED_UPIU_SIZE);
		} else {
			utrdlp[i].response_upiu_offset = cpu_to_le16((response_offset >> 2));
			utrdlp[i].prd_table_offset = cpu_to_le16((prdt_offset >> 2));
			utrdlp[i].response_upiu_length = cpu_to_le16(ALIGNED_UPIU_SIZE >> 2);
		}

		hba->lrb[i].utr_descriptor_ptr = (utrdlp + i);
		hba->lrb[i].utrd_dma_addr = hba->utrdl_dma_addr + (i * sizeof(struct utp_transfer_req_desc));
		hba->lrb[i].ucd_req_ptr = (struct utp_upiu_req *)(cmd_descp + i);
		hba->lrb[i].ucd_req_dma_addr = cmd_desc_element_addr;
		hba->lrb[i].ucd_rsp_ptr = (struct utp_upiu_rsp *)cmd_descp[i].response_upiu;
		hba->lrb[i].ucd_rsp_dma_addr = cmd_desc_element_addr + response_offset;
		hba->lrb[i].ucd_prdt_ptr = (struct ufshcd_sg_entry *)cmd_descp[i].prd_table;
		hba->lrb[i].ucd_prdt_dma_addr = cmd_desc_element_addr + prdt_offset;
	}
	pr_info("NULL_DEV: null_ufshcd_host_memory_configure(), completed\n");
}

static void null_ufshcd_err_handler(struct work_struct *work)
{
	pr_info("NULL_DEV: null_ufshcd_err_handler(), started\n");
	pr_info("NULL_DEV: null_ufshcd_err_handler(), completed\n");
}

static void null_ufshcd_exception_event_handler(struct work_struct *work)
{
	pr_info("NULL_DEV: null_ufshcd_exception_event_handler(), started\n");
	pr_info("NULL_DEV: null_ufshcd_exception_event_handler(), completed\n");
}

static void null_ufshcd_gate_work(struct work_struct *work){}
static void null_ufshcd_ungate_work(struct work_struct *work){}
static ssize_t null_ufshcd_clkgate_delay_show(struct device *dev, struct device_attribute *attr, char *buf) { return 0; }
static ssize_t null_ufshcd_clkgate_delay_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { return 0; }
static ssize_t null_ufshcd_clkgate_enable_show(struct device *dev, struct device_attribute *attr, char *buf) { return 0; }
static ssize_t null_ufshcd_clkgate_enable_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) { return 0; }
static void null_ufshcd_check_errors(struct ufs_hba *hba) {}


static void null_ufshcd_init_clk_gating(struct ufs_hba *hba)
{
	pr_info("NULL_DEV: null_ufshcd_init_clk_gating(), started\n");
	if (!(hba->caps & UFSHCD_CAP_CLK_GATING))
		return;
	hba->clk_gating.delay_ms = 150;
	INIT_DELAYED_WORK(&hba->clk_gating.gate_work, null_ufshcd_gate_work);
	INIT_WORK(&hba->clk_gating.ungate_work, null_ufshcd_ungate_work);

	hba->clk_gating.is_enabled = true;
	
	hba->clk_gating.delay_attr.show = null_ufshcd_clkgate_delay_show;
	hba->clk_gating.delay_attr.store = null_ufshcd_clkgate_delay_store;
	sysfs_attr_init(&hba->clk_gating.delay_attr.attr);
	hba->clk_gating.delay_attr.attr.name = "clkgate_delay_ms";
	hba->clk_gating.delay_attr.attr.mode = 0644;
	
	if (device_create_file(hba->dev, &hba->clk_gating.delay_attr))
		pr_info("NULL_DEV: null_ufshcd_init_clk_gating(), failed to create sysfs for clkgate_delay\n");
	
	hba->clk_gating.enable_attr.show = null_ufshcd_clkgate_enable_show;
	hba->clk_gating.enable_attr.store = null_ufshcd_clkgate_enable_store;
	sysfs_attr_init(&hba->clk_gating.enable_attr.attr);
	hba->clk_gating.enable_attr.attr.name = "clkgate_enable";
	hba->clk_gating.enable_attr.attr.mode = 0644;
	
	if (device_create_file(hba->dev, &hba->clk_gating.enable_attr))
		pr_info("NULL_DEV: null_ufshcd_init_clk_gating(), failed to create sysfs for clkgate_enable\n");
	pr_info("NULL_DEV: null_ufshcd_init_clk_gating(), completed\n");
}

static irqreturn_t null_ufshcd_intr(int irq, void *__hba)
{
	u32 intr_status, enabled_intr_status;
	irqreturn_t retval = IRQ_NONE;
	struct ufs_hba *hba = __hba;

	pr_info("NULL_DEV: null_ufshcd_intr(), started\n");
	spin_lock(hba->host->host_lock);
	intr_status = ufshcd_readl(hba, REG_INTERRUPT_STATUS);
	enabled_intr_status = intr_status & ufshcd_readl(hba, REG_INTERRUPT_ENABLE);
	if (intr_status)
		ufshcd_writel(hba, intr_status, REG_INTERRUPT_STATUS);
	if (enabled_intr_status) {
		hba->errors = UFSHCD_ERROR_MASK & enabled_intr_status;

		if (hba->errors)
			null_ufshcd_check_errors(hba);
		
		if (enabled_intr_status & UFSHCD_UIC_MASK) {
			if ((enabled_intr_status & UIC_COMMAND_COMPL) && hba->active_uic_cmd) {
				hba->active_uic_cmd->argument2 |= (ufshcd_readl(hba, REG_UIC_COMMAND_ARG_2) & MASK_UIC_COMMAND_RESULT);
				hba->active_uic_cmd->argument3 = ufshcd_readl(hba, REG_UIC_COMMAND_ARG_3);
				complete(&hba->active_uic_cmd->done);
			}
			if ((enabled_intr_status & UFSHCD_UIC_PWR_MASK) && hba->uic_async_done)
				complete(hba->uic_async_done);
		}

		if (enabled_intr_status & UTP_TASK_REQ_COMPL) {
			hba->tm_condition = ufshcd_readl(hba, REG_UTP_TASK_REQ_DOOR_BELL) ^ hba->outstanding_tasks;
			wake_up(&hba->tm_wq);
		}

		if (enabled_intr_status & UTP_TRANSFER_REQ_COMPL)
			//ufshcd_transfer_req_compl(hba);
		retval = IRQ_HANDLED;
	}
	spin_unlock(hba->host->host_lock);
	pr_info("NULL_DEV: null_ufshcd_intr(), completed\n");
	return retval;
}


static int null_ufshcd_register(void) 
{
	struct ufs_hba *hba;
	struct Scsi_Host *host;
	struct device *dev;
	void __iomem *mmio_base;
	unsigned int irq = 15;
	int err = 0;

	pr_info("NULL_DEV: null_ufshcd_register(), started\n");
	/* alloc null device */
	dev = kzalloc(sizeof(struct device), GFP_KERNEL);
	if (!dev) {
		pr_info("NULL_DEV: null_ufshcd_register(), kzalloc device failed\n");
		err = -ENODEV;
		goto out_error;
	}

	/* alloc Scsi_Host */
	host = scsi_host_alloc(&null_ufshcd_driver_template, sizeof(struct ufs_hba));
	if (!host) {
		pr_info("NULL_DEV: null_ufshcd_register(), scsi_host_alloc failed\n");
		err = -ENOMEM;
		goto out_error;
	}
	hba = shost_priv(host);
	hba->host = host;
	hba->dev = dev;
	
	INIT_LIST_HEAD(&hba->clk_list_head);

	/* mmio_base init  260 bytes */
	mmio_base = kzalloc(260, GFP_KERNEL);
	if (!mmio_base) {
		pr_info("NULL_DEV: null_ufshcd_register(), kzalloc mmio_base failed\n");
		err = -ENODEV;
		goto out_error;
	}
	hba->mmio_base = mmio_base;
	hba->irq = irq;

	/* Init registers */
#define __NOTSET__ 0x0000

	ufshcd_writel(hba, 0x0707101f, REG_CONTROLLER_CAPABILITIES);
	ufshcd_writel(hba, 0x00000210, REG_UFS_VERSION);						/* UFS Version */
	ufshcd_writel(hba, 0xeeee, REG_CONTROLLER_DEV_ID);					/* Product ID */
	ufshcd_writel(hba, 0xeeee, REG_CONTROLLER_PROD_ID);						/* Manufacturer ID */
	ufshcd_writel(hba, __NOTSET__, REG_AUTO_HIBERNATE_IDLE_TIMER);
	ufshcd_writel(hba, __NOTSET__, REG_INTERRUPT_STATUS);
	ufshcd_writel(hba, __NOTSET__, REG_INTERRUPT_ENABLE);
	ufshcd_writel(hba, 0x000f, REG_CONTROLLER_STATUS);
	ufshcd_writel(hba, 0x0001, REG_CONTROLLER_ENABLE);
	ufshcd_writel(hba, __NOTSET__, REG_UIC_ERROR_CODE_PHY_ADAPTER_LAYER);
	ufshcd_writel(hba, __NOTSET__, REG_UIC_ERROR_CODE_DATA_LINK_LAYER);
	ufshcd_writel(hba, __NOTSET__, REG_UIC_ERROR_CODE_NETWORK_LAYER);
	ufshcd_writel(hba, __NOTSET__, REG_UIC_ERROR_CODE_TRANSPORT_LAYER);
	ufshcd_writel(hba, __NOTSET__, REG_UIC_ERROR_CODE_DME);
	ufshcd_writel(hba, __NOTSET__, REG_UTP_TRANSFER_REQ_INT_AGG_CONTROL);
	ufshcd_writel(hba, 0x8000, REG_UTP_TRANSFER_REQ_LIST_BASE_L);
	ufshcd_writel(hba, 0xb000, REG_UTP_TRANSFER_REQ_LIST_BASE_H);
	ufshcd_writel(hba, __NOTSET__, REG_UTP_TRANSFER_REQ_DOOR_BELL);
	ufshcd_writel(hba, __NOTSET__, REG_UTP_TRANSFER_REQ_LIST_CLEAR);
	ufshcd_writel(hba, __NOTSET__, REG_UTP_TRANSFER_REQ_LIST_RUN_STOP);
	ufshcd_writel(hba, 0xe000, REG_UTP_TASK_REQ_LIST_BASE_L);
	ufshcd_writel(hba, 0xf800, REG_UTP_TASK_REQ_LIST_BASE_H);
	ufshcd_writel(hba, __NOTSET__, REG_UTP_TASK_REQ_DOOR_BELL);
	ufshcd_writel(hba, __NOTSET__, REG_UTP_TASK_REQ_LIST_CLEAR);
	ufshcd_writel(hba, __NOTSET__, REG_UTP_TASK_REQ_LIST_RUN_STOP);
	ufshcd_writel(hba, __NOTSET__, REG_UIC_COMMAND);
	ufshcd_writel(hba, __NOTSET__, REG_UIC_COMMAND_ARG_1);
	ufshcd_writel(hba, __NOTSET__, REG_UIC_COMMAND_ARG_2);
	ufshcd_writel(hba, __NOTSET__, REG_UIC_COMMAND_ARG_3);
	ufshcd_writel(hba, __NOTSET__, UFSHCI_REG_SPACE_SIZE);
	ufshcd_writel(hba, __NOTSET__, REG_UFS_CCAP);

	pr_info("NULL_DEV: null_ufshcd_register(), [REG_CONTROLLER_CAPABILITIES]=%x\n", 
		ufshcd_readl(hba, REG_CONTROLLER_CAPABILITIES));
	pr_info("NULL_DEV: null_ufshcd_register(), [REG_UFS_VERSION]=%x\n", 
		ufshcd_readl(hba, REG_UFS_VERSION));
	pr_info("NULL_DEV: null_ufshcd_register(), [REG_CONTROLLER_DEV_ID]=%x\n", 
		ufshcd_readl(hba, REG_CONTROLLER_DEV_ID));
	pr_info("NULL_DEV: null_ufshcd_register(), [REG_CONTROLLER_PROD_ID]=%x\n", 
		ufshcd_readl(hba, REG_CONTROLLER_PROD_ID));
	pr_info("NULL_DEV: null_ufshcd_register(), [REG_AUTO_HIBERNATE_IDLE_TIMER]=%x\n", 
		ufshcd_readl(hba, REG_AUTO_HIBERNATE_IDLE_TIMER));
	pr_info("NULL_DEV: null_ufshcd_register(), [REG_INTERRUPT_STATUS]=%x\n", 
		ufshcd_readl(hba, REG_INTERRUPT_STATUS));
	pr_info("NULL_DEV: null_ufshcd_register(), [REG_INTERRUPT_ENABLE]=%x\n", 
		ufshcd_readl(hba, REG_INTERRUPT_ENABLE));
	pr_info("NULL_DEV: null_ufshcd_register(), [REG_CONTROLLER_STATUS]=%x\n", 
		ufshcd_readl(hba, REG_CONTROLLER_STATUS));
	pr_info("NULL_DEV: null_ufshcd_register(), [REG_CONTROLLER_ENABLE]=%x\n", 
		ufshcd_readl(hba, REG_CONTROLLER_ENABLE));
	pr_info("NULL_DEV: null_ufshcd_register(), [REG_UIC_ERROR_CODE_PHY_ADAPTER_LAYER]=%x\n", 
		ufshcd_readl(hba, REG_UIC_ERROR_CODE_PHY_ADAPTER_LAYER));
	pr_info("NULL_DEV: null_ufshcd_register(), [REG_UIC_ERROR_CODE_DATA_LINK_LAYER]=%x\n", 
		ufshcd_readl(hba, REG_UIC_ERROR_CODE_DATA_LINK_LAYER));
	pr_info("NULL_DEV: null_ufshcd_register(), [REG_UIC_ERROR_CODE_NETWORK_LAYER]=%x\n", 
		ufshcd_readl(hba, REG_UIC_ERROR_CODE_NETWORK_LAYER));
	pr_info("NULL_DEV: null_ufshcd_register(), [REG_UIC_ERROR_CODE_TRANSPORT_LAYER]=%x\n", 
		ufshcd_readl(hba, REG_UIC_ERROR_CODE_TRANSPORT_LAYER));
	pr_info("NULL_DEV: null_ufshcd_register(), [REG_UIC_ERROR_CODE_DME]=%x\n", 
		ufshcd_readl(hba, REG_UIC_ERROR_CODE_DME));
	pr_info("NULL_DEV: null_ufshcd_register(), [REG_UTP_TRANSFER_REQ_INT_AGG_CONTROL]=%x\n", 
		ufshcd_readl(hba, REG_UTP_TRANSFER_REQ_INT_AGG_CONTROL));
	pr_info("NULL_DEV: null_ufshcd_register(), [REG_UTP_TRANSFER_REQ_LIST_BASE_L]=%x\n", 
		ufshcd_readl(hba, REG_UTP_TRANSFER_REQ_LIST_BASE_L));
	pr_info("NULL_DEV: null_ufshcd_register(), [REG_UTP_TRANSFER_REQ_LIST_BASE_H]=%x\n", 
		ufshcd_readl(hba, REG_UTP_TRANSFER_REQ_LIST_BASE_H));
	pr_info("NULL_DEV: null_ufshcd_register(), [REG_UTP_TRANSFER_REQ_DOOR_BELL]=%x\n", 
		ufshcd_readl(hba, REG_UTP_TRANSFER_REQ_DOOR_BELL));
	pr_info("NULL_DEV: null_ufshcd_register(), [REG_UTP_TASK_REQ_LIST_CLEAR]=%x\n", 
		ufshcd_readl(hba, REG_UTP_TASK_REQ_LIST_CLEAR));
	pr_info("NULL_DEV: null_ufshcd_register(), [REG_UTP_TASK_REQ_LIST_RUN_STOP]=%x\n", 
		ufshcd_readl(hba, REG_UTP_TASK_REQ_LIST_RUN_STOP));
	pr_info("NULL_DEV: null_ufshcd_register(), [REG_UIC_COMMAND]=%x\n", 
		ufshcd_readl(hba, REG_UIC_COMMAND));
	pr_info("NULL_DEV: null_ufshcd_register(), [REG_UIC_COMMAND_ARG_1]=%x\n", 
		ufshcd_readl(hba, REG_UIC_COMMAND_ARG_1));
	pr_info("NULL_DEV: null_ufshcd_register(), [REG_UIC_COMMAND_ARG_2]=%x\n", 
		ufshcd_readl(hba, REG_UIC_COMMAND_ARG_2));
	pr_info("NULL_DEV: null_ufshcd_register(), [REG_UIC_COMMAND_ARG_3]=%x\n", 
		ufshcd_readl(hba, REG_UIC_COMMAND_ARG_3));
	pr_info("NULL_DEV: null_ufshcd_register(), [UFSHCI_REG_SPACE_SIZE]=%x\n", 
		ufshcd_readl(hba, UFSHCI_REG_SPACE_SIZE));
	pr_info("NULL_DEV: null_ufshcd_register(), [REG_UFS_CCAP]=%x\n", 
		ufshcd_readl(hba, REG_UFS_CCAP));

	/* Set descriptor lengths to specification defaults */
	hba->desc_size.dev_desc = QUERY_DESC_DEVICE_DEF_SIZE;
	hba->desc_size.pwr_desc = QUERY_DESC_POWER_DEF_SIZE;
	hba->desc_size.interc_desc = QUERY_DESC_INTERCONNECT_DEF_SIZE;
	hba->desc_size.conf_desc = QUERY_DESC_CONFIGURATION_DEF_SIZE;
	hba->desc_size.unit_desc = QUERY_DESC_UNIT_DEF_SIZE;
	hba->desc_size.geom_desc = QUERY_DESC_GEOMETRY_DEF_SIZE;

	/* Host bus adapter init */
	/////////////////////////////////////////////////////
	
	/* Read capabilities registers */
	hba->capabilities = ufshcd_readl(hba, REG_CONTROLLER_CAPABILITIES);
	hba->nutrs = (hba->capabilities & MASK_TRANSFER_REQUESTS_SLOTS) + 1;
	hba->nutmrs = ((hba->capabilities & MASK_TASK_MANAGEMENT_REQUEST_SLOTS) >> 16) + 1;

	/* Get UFS version supported by the controller */
	hba->ufs_version = ufshcd_readl(hba, REG_UFS_VERSION);
	if ((hba->ufs_version != UFSHCI_VERSION_10) &&
	    (hba->ufs_version != UFSHCI_VERSION_11) &&
	    (hba->ufs_version != UFSHCI_VERSION_20) &&
	    (hba->ufs_version != UFSHCI_VERSION_21))
	    pr_info("NULL_DEV: null_ufshcd_register(), invalid UFS version\n");

	/* Get Interrupt bit mask per version */
	switch (hba->ufs_version) {
	case UFSHCI_VERSION_10:
		hba->intr_mask = INTERRUPT_MASK_ALL_VER_10;
		break;
	case UFSHCI_VERSION_11:
	case UFSHCI_VERSION_20:
		hba->intr_mask = INTERRUPT_MASK_ALL_VER_11;
		break;
	case UFSHCI_VERSION_21:
	default:
		hba->intr_mask = INTERRUPT_MASK_ALL_VER_21;
		break;
	}

	
	// Set DMA mask 
	hba->dev->dma_mask = kzalloc(sizeof(u64), GFP_KERNEL);
	*(hba->dev->dma_mask) = 0xffffffff;
	hba->dev->coherent_dma_mask = 0xffffffff;
	/*
	err = dma_set_mask_and_coherent(hba->dev, DMA_BIT_MASK(32));
	if (err) {
		pr_info("NULL_DEV: null_ufshcd_register(), set dma mask failed\n");
		goto out_error;
	}
		
	// Allocate memory for host memory space 
	
	err = null_ufshcd_memory_alloc(hba);
	if (err) {
		pr_info("NULL_DEV: null_ufshcd_register(), memory allocation failed\n");
		goto out_error;
	}
	
	// Configure LRB 
	null_ufshcd_host_memory_configure(hba);
	*/
	host->can_queue = hba->nutrs;
	host->cmd_per_lun = hba->nutrs;
	host->max_id = 1;
	host->max_lun = UFS_MAX_LUNS;
	host->max_channel = 0;
	host->unique_id = host->host_no;
	host->max_cmd_len = MAX_CDB_SIZE;
	hba->max_pwr_info.is_valid = false;
	pr_info("NULL_DEV: null_ufshcd_register(), host attr inited\n");

	/* Initailize wait queue for task management */
	init_waitqueue_head(&hba->tm_wq);
	init_waitqueue_head(&hba->tm_tag_wq);
	
	/* Initialize work queues */
	INIT_WORK(&hba->eh_work, null_ufshcd_err_handler);
	INIT_WORK(&hba->eeh_work, null_ufshcd_exception_event_handler);

	/* Initialize UIC command mutex */
	mutex_init(&hba->uic_cmd_mutex);

	/* Initialize mutex for device management commands */
	mutex_init(&hba->dev_cmd.lock);
	init_rwsem(&hba->clk_scaling_lock);
	
	/* Initialize device management tag acquire wait queue */
	init_waitqueue_head(&hba->dev_cmd.tag_wq);
	pr_info("NULL_DEV: null_ufshcd_register(), waitqueues workqueues and locks inited\n");
	
	//null_ufshcd_init_clk_gating(hba);

	ufshcd_writel(hba, ufshcd_readl(hba, REG_INTERRUPT_STATUS), REG_INTERRUPT_STATUS);
	ufshcd_writel(hba, 0, REG_INTERRUPT_ENABLE);
	mb();

	/* IRQ registration */
	//err = devm_request_irq(dev, irq, null_ufshcd_intr, IRQF_SHARED, UFSHCD, hba);
	if (err) {
		pr_info("NULL_DEV: null_ufshcd_register(), request irq failed\n");
		goto out_error;
	} else {
		hba->is_irq_enabled = true;
	}
	
	err = scsi_add_host(host, hba->dev);
	if (err) {
		pr_info("NULL_DEV: null_ufshcd_register(), scsi_add_host failed\n");
		goto out_error;
	}

	/* Host controller enable */

	null_hba = hba;
	pr_info("NULL_DEV: null_ufshcd_register(), completed\n");
	
out_error:
	pr_info("NULL_DEV: null_ufshcd_register(), failed\n");
	return err;
}

static void null_ufshcd_unregister(void) { /* kfree null_hba */ }

#else
static int null_ufshcd_register(void) 
{
	pr_info("NULL_DEV: null_ufshcd_register(), CONFIG_SCSI needs to be enabled for UFSHCD\n");
	return -EINVAL;
}

static void null_ufshcd_unregister(void) {}
#endif /* CONFIG_SCSI */

static int __init nulldev_init(void) 
{
	pr_info("NULL_DEV: nulldev_init()\n");
	return null_ufshcd_register();
}

static void __exit nulldev_exit(void)
{
	pr_info("NULL_DEV: nulldev_exit()\n");
	null_ufshcd_unregister();
}

module_init(nulldev_init);
module_exit(nulldev_exit);

MODULE_AUTHOR("Xiong Xiong <xxiong@cqu.edu.cn>");
MODULE_LICENSE("GPL");

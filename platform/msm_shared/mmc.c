/* Copyright (c) 2010, Code Aurora Forum. All rights reserved.

 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above
 *     copyright notice, this list of conditions and the following
 *     disclaimer in the documentation and/or other materials provided
 *     with the distribution.
 *   * Neither the name of Code Aurora Forum, Inc. nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
 * IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */  
    
#include <string.h>
#include <stdlib.h>
#include <debug.h>
#include <reg.h>
#include "mmc.h"
#include <platform/iomap.h>
    
#ifndef NULL
#define NULL        0
#endif				/* 
    
#define ROUND_TO_PAGE(x,y) (((x) + (y)) & (~(y)))
    
/* data access time unit in ns */ 
static const unsigned int taac_unit[] = 
    { 1, 10, 100, 1000, 10000, 100000, 1000000, 10000000 };

/* data access time value x 10 */ 
static const unsigned int taac_value[] = 
    { 0, 10, 12, 13, 15, 20, 25, 30, 35, 40, 45, 50, 55, 60, 70, 80 };

/* data transfer rate in kbit/s */ 
static const unsigned int xfer_rate_unit[] = 
    { 100, 1000, 10000, 100000, 0, 0, 0, 0 };

/* data transfer rate value x 10*/ 
static const unsigned int xfer_rate_value[] = 
    { 0, 10, 12, 13, 15, 20, 26, 30, 35, 40, 45, 52, 55, 60, 70, 80 };







    { SDC1_PCLK, SDC2_PCLK, SDC3_PCLK, SDC4_PCLK };














		       unsigned int data_len);

			    unsigned char set_clear_wp);

					   unsigned char *buf);

{
	
	    (((val >> 8) & 0xFF) << 16) | 
									   >>
									   24);



/*
 * Function to enable and set master and peripheral clock for
 * MMC card.
 */ 
static unsigned int
mmc_boot_enable_clock(struct mmc_boot_host *host, 
{
	
	
#ifndef PLATFORM_MSM8X60
	int mmc_signed_ret = 0;
	
	
	
		
		
		
	
		
		
		    /* set clock */ 
		    if (mmc_clock_enable_disable(SDC_PCLK, MMC_CLK_ENABLE) < 0)
			
			
			
			
		
			
			
			
			
		
		
	
		
		
			
			
				 "Failure setting clock rate for MCLK - clk_rate: %d\n!",
				 mclk);
			
			
		
			
			
				 "Failure getting clock rate for MCLK - clk_rate: %d\n!",
				 host->mclk_rate);
			
			
		
		
	
		
		
			 "Failure getting clock rate for PCLK - clk_rate: %d\n!",
			 host->pclk_rate);
		
		
	
	
		 host->mclk_rate, host->pclk_rate);
	
#else				/* 
	    clock_set_enable(mclk);
	
	
	
	
#endif				/* 
	    //enable mci clock
	    mmc_clk |= MMC_BOOT_MCI_CLK_ENABLE;
	
	    //enable flow control
	    mmc_clk |= MMC_BOOT_MCI_CLK_ENA_FLOW;
	
	    //latch data and command using feedback clock
	    mmc_clk |= MMC_BOOT_MCI_CLK_IN_FEEDBACK;
	
	
	
#ifndef PLATFORM_MSM8X60
 error_pclk:
 
 
	
#endif				/* 
}


/* Sets a timeout for read operation.
 */ 
static unsigned int
mmc_boot_set_read_timeout(struct mmc_boot_host *host,
			  
{
	
	
		
		
		
	
	       || (card->type == MMC_BOOT_TYPE_SDHC))
		
		
		
	
	else if ((card->type == MMC_BOOT_TYPE_STD_SD)
		 || (card->type == MMC_BOOT_TYPE_STD_MMC))
		
		
				    (card->csd.nsac_clk_cycle /
				     (host->mclk_rate / 1000000000)));
		
		
	
	else
		
		
		
	
	



/* Sets a timeout for write operation.
 */ 
static unsigned int
mmc_boot_set_write_timeout(struct mmc_boot_host *host,
			   
{
	
	
		
		
		
	
	       || (card->type == MMC_BOOT_TYPE_SDHC))
		
		
		
	
	else if (card->type == MMC_BOOT_TYPE_STD_SD
		 || (card->type == MMC_BOOT_TYPE_STD_MMC))
		
		
				    (card->csd.nsac_clk_cycle /
				     (host->mclk_rate / 1000000000)));
		
		
		
	
	else
		
		
		
	
	



/*
 * Decodes CSD response received from the card. Note that we have defined only
 * few of the CSD elements in csd structure. We'll only decode those values.
 */ 
static unsigned int
mmc_boot_decode_and_save_csd(struct mmc_boot_card *card,
			     
{
	
	
	
	
	
	
		
		
		
	
	
	
	       || (card->type == MMC_BOOT_TYPE_STD_SD))
		
		
		    /* Parse CSD according to SD card spec. */ 
		    
		    /* CSD register is little bit differnet for CSD version 2.0 High Capacity
		     * and CSD version 1.0/2.0 Standard memory cards. In Version 2.0 some of
		     * the fields have fixed values and it's not necessary for host to refer
		     * these fields in CSD sent by card */ 
		    
			
			
			    /* CSD Version 2.0 */ 
			    mmc_csd.card_cmd_class =
			    UNPACK_BITS(raw_csd, 84, 12, mmc_sizeof);
			
			
			
			
			
			    UNPACK_BITS(raw_csd, 48, 22, mmc_sizeof);
			
			    UNPACK_BITS(raw_csd, 104, 8, mmc_sizeof) * 100;
			
//TODO: Investigate the nsac and taac. Spec suggests not using this for timeouts.
			    
			    UNPACK_BITS(raw_csd, 112, 3, mmc_sizeof);
			
			
			    (taac_value[mmc_value] * taac_unit[mmc_unit]) / 10;
			
			
			
			
			
			
			
			
			    (xfer_rate_value[mmc_value] *
			     xfer_rate_unit[mmc_unit]) / 10;
			
			
			
			    UNPACK_BITS(raw_csd, 13, 1, mmc_sizeof);
			
			    UNPACK_BITS(raw_csd, 12, 1, mmc_sizeof);
			
			    /* Calculate the card capcity */ 
			    card->capacity = (1 + mmc_csd.c_size) * 512 * 1024;
			
		
		else
			
			
			    /* CSD Version 1.0 */ 
			    mmc_csd.card_cmd_class =
			    UNPACK_BITS(raw_csd, 84, 12, mmc_sizeof);
			
			
						  && mmc_temp <
						  12) ? (1 << mmc_temp) : 512;
			
			
						 && mmc_temp <
						 12) ? (1 << mmc_temp) : 512;
			
			
			
			    (taac_value[mmc_value] * taac_unit[mmc_unit]) / 10;
			
			
			
			    (xfer_rate_value[mmc_value] *
			     xfer_rate_unit[mmc_unit]) / 10;
			
			    UNPACK_BITS(raw_csd, 104, 8, mmc_sizeof) * 100;
			
			    UNPACK_BITS(raw_csd, 26, 3, mmc_sizeof);
			
			    UNPACK_BITS(raw_csd, 39, 7, mmc_sizeof) + 1;
			
			    UNPACK_BITS(raw_csd, 46, 1, mmc_sizeof);
			
			    UNPACK_BITS(raw_csd, 77, 1, mmc_sizeof);
			
			    UNPACK_BITS(raw_csd, 78, 1, mmc_sizeof);
			
			    UNPACK_BITS(raw_csd, 79, 1, mmc_sizeof);
			
			    UNPACK_BITS(raw_csd, 21, 1, mmc_sizeof);
			
			    UNPACK_BITS(raw_csd, 47, 3, mmc_sizeof);
			
			    UNPACK_BITS(raw_csd, 62, 12, mmc_sizeof);
			
			    UNPACK_BITS(raw_csd, 32, 7, mmc_sizeof);
			
			    UNPACK_BITS(raw_csd, 31, 1, mmc_sizeof);
			
			    UNPACK_BITS(raw_csd, 13, 1, mmc_sizeof);
			
			    UNPACK_BITS(raw_csd, 12, 1, mmc_sizeof);
			
			    /* Calculate the card capacity */ 
			    mmc_temp =
			    (1 << (mmc_csd.c_size_mult + 2)) * (mmc_csd.c_size +
								1);
			
			
		
	
	else
		
		
		    /* Parse CSD according to MMC card spec. */ 
		    mmc_csd.spec_vers =
		    UNPACK_BITS(raw_csd, 122, 4, mmc_sizeof);
		
		    UNPACK_BITS(raw_csd, 84, 12, mmc_sizeof);
		
		    1 << UNPACK_BITS(raw_csd, 22, 4, mmc_sizeof);
		
		    1 << UNPACK_BITS(raw_csd, 80, 4, mmc_sizeof);
		
		
		
		
		    UNPACK_BITS(raw_csd, 104, 8, mmc_sizeof) * 100;
		
		
		
		    (taac_value[mmc_value] * taac_unit[mmc_unit]) / 10;
		
		    UNPACK_BITS(raw_csd, 77, 1, mmc_sizeof);
		
		    UNPACK_BITS(raw_csd, 78, 1, mmc_sizeof);
		
		    UNPACK_BITS(raw_csd, 79, 1, mmc_sizeof);
		
		    UNPACK_BITS(raw_csd, 21, 1, mmc_sizeof);
		
		
		    UNPACK_BITS(raw_csd, 42, 5, mmc_sizeof);
		
		    UNPACK_BITS(raw_csd, 37, 5, mmc_sizeof);
		
		
		    UNPACK_BITS(raw_csd, 31, 1, mmc_sizeof);
		
		
		
		    /* Calculate the card capcity */ 
		    if (mmc_csd.c_size != 0xFFF)
			
			
			    /* For cards less than or equal to 2GB */ 
			    mmc_temp =
			    (1 << (mmc_csd.c_size_mult + 2)) * (mmc_csd.c_size +
								1);
			
			
		
		else
			
			
			    /* For cards greater than 2GB, Ext CSD register's SEC_COUNT
			     * is used to calculate the size.
			     */ 
			unsigned long long sec_count;
			
			    (ext_csd_buf[214] << 16) | 
			    (ext_csd_buf[213] << 8) | 
			
		
	    /* save the information in card structure */ 
	    memcpy((struct mmc_boot_csd *)&card->csd,
		   (struct mmc_boot_csd *)&mmc_csd,
		   
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	



/*
 * Decode CID sent by the card.
 */ 
static unsigned int
mmc_boot_decode_and_save_cid(struct mmc_boot_card *card,
			     
{
	
	
	
	
		
		
		
	
	
	
	
		
		
							     (104 -
							      8 * (i + 1)), 8,
							     mmc_sizeof);
		
	
	
	
	
	
	    /* save it in card database */ 
	    memcpy((struct mmc_boot_cid *)&card->cid, \
		   (struct mmc_boot_cid *)&mmc_cid,
		   \
	
	
	
	
	
		 (mmc_cid.prv & 0xF));
	
	
		 mmc_cid.year + 1997);
	



/*
 * Sends specified command to a card and waits for a response.
 */ 
static unsigned int mmc_boot_send_command(struct mmc_boot_command *cmd) 
{
	
	
	
	
	
	
	
	    /* basic check */ 
	    if (cmd == NULL)
		
		
		
	
	    /* 1. Write command argument to MMC_BOOT_MCI_ARGUMENT register */ 
	    writel(cmd->argument, MMC_BOOT_MCI_ARGUMENT);
	
	    /* Writes to MCI port are not effective for 3 ticks of PCLK.
	     * The min pclk is 144KHz which gives 6.94 us/tick.
	     * Thus 21us == 3 ticks.
	     */ 
	    udelay(21);
	
	    /* 2. Set appropriate fields and write MMC_BOOT_MCI_CMD */ 
	    /* 2a. Write command index in CMD_INDEX field */ 
	    cmd_index = cmd->cmd_index;
	
	
	    /* 2b. Set RESPONSE bit to 1 for all cmds except CMD0 */ 
	    if (cmd_index != CMD0_GO_IDLE_STATE)
		
		
		
	
	    /* 2c. Set LONGRESP bit to 1 for CMD2, CMD9 and CMD10 */ 
	    if (IS_RESP_136_BITS(cmd->resp_type))
		
		
		
	
	    /* 2d. Set INTERRUPT bit to 1 to disable command timeout */ 
	    
	    /* 2e. Set PENDING bit to 1 for CMD12 in the beginning of stream
	       mode data transfer */ 
	    if (cmd->xfer_mode == MMC_BOOT_XFER_MODE_STREAM)
		
		
		
	
	    /* 2f. Set ENABLE bit to 1 */ 
	    mmc_cmd |= MMC_BOOT_MCI_CMD_ENABLE;
	
	    /* 2g. Set PROG_ENA bit to 1 for CMD12, CMD13 issued at the end of
	       write data transfer */ 
	    if ((cmd_index == CMD12_STOP_TRANSMISSION
		 || 
		
		
		
	
	    /* 2h. Set MCIABORT bit to 1 for CMD12 when working with SDIO card */ 
	    /* 2i. Set CCS_ENABLE bit to 1 for CMD61 when Command Completion Signal
	       of CE-ATA device is enabled */ 
	    
	    /* 2j. clear all static status bits */ 
	    writel(MMC_BOOT_MCI_STATIC_STATUS, MMC_BOOT_MCI_CLEAR);
	
	    /* 2k. Write to MMC_BOOT_MCI_CMD register */ 
	    writel(mmc_cmd, MMC_BOOT_MCI_CMD);
	
		   
	
	    /* 3. Wait for interrupt or poll on the following bits of MCI_STATUS
	       register */ 
	    do {
		
		    /* 3a. Read MCI_STATUS register */ 
		    while (readl(MMC_BOOT_MCI_STATUS) \
			   
		
		
		    /* 3b. CMD_SENT bit supposed to be set to 1 only after CMD0 is sent -
		       no response required. */ 
		    if ((cmd->resp_type == MMC_BOOT_RESP_NONE) && 
			(mmc_status & MMC_BOOT_MCI_STAT_CMD_SENT))
			
			
			
		
		    /* 3c. If CMD_TIMEOUT bit is set then no response was received */ 
		    else if (mmc_status & MMC_BOOT_MCI_STAT_CMD_TIMEOUT)
			
			
			
			
		
		    /* 3d. If CMD_RESPONSE_END bit is set to 1 then command's response was
		       received and CRC check passed
		       Spcial case for ACMD41: it seems to always fail CRC even if
		       the response is valid
		     */ 
		    else if ((mmc_status & MMC_BOOT_MCI_STAT_CMD_RESP_END)
			     || (cmd_index ==
				 CMD1_SEND_OP_COND) 
							 CMD8_SEND_IF_COND))
			
			
			    /* 3i. Read MCI_RESP_CMD register to verify that response index is
			       equal to command index */ 
			    mmc_resp = readl(MMC_BOOT_MCI_RESP_CMD) & 0x3F;
			
			    /* However, long response does not contain the command index field.
			     * In that case, response index field must be set to 111111b (0x3F) */ 
			    if ((mmc_resp == cmd_index) || 
				(cmd->resp_type == MMC_BOOT_RESP_R2
				 || 
				 || 
				 || 
				
				
				    /* 3j. If resp index is equal to cmd index, read command resp
				       from MCI_RESPn registers
				       - MCI_RESP0/1/2/3 for CMD2/9/10
				       - MCI_RESP0 for all other registers */ 
				    if (IS_RESP_136_BITS(cmd->resp_type))
					
					
						
						
						    readl(MMC_BOOT_MCI_RESP_0 +
							  (i * 4));
						
					
				
				else
					
					
					    readl(MMC_BOOT_MCI_RESP_0);
					
				
			
			else
				
				
				    /* command index mis-match */ 
				    mmc_return = MMC_BOOT_E_CMD_INDX_MISMATCH;
				
			
				   cmd->resp[0]);
			
			
		
		    /* 3e. If CMD_CRC_FAIL bit is set to 1 then cmd's response was recvd,
		       but CRC check failed. */ 
		    else if ((mmc_status & MMC_BOOT_MCI_STAT_CMD_CRC_FAIL))
			
			
				
				
				
			
			else
				
			
			
	
	while (1);
	



/*
 * Reset all the cards to idle condition (CMD 0)
 */ 
static unsigned int mmc_boot_reset_cards(void) 
{
	
	
		  
	
	
	cmd.cmd_type = MMC_BOOT_CMD_BCAST;
	
	
	    /* send command */ 
	    return mmc_boot_send_command(&cmd);



/*
 * Send CMD1 to know whether the card supports host VDD profile or not.
 */ 
static unsigned int
mmc_boot_send_op_cond(struct mmc_boot_host *host, 
{
	
	
	
	
	    /* basic check */ 
	    if ((host == NULL) || (card == NULL))
		
		
		
	
		  
	
	    /* CMD1 format:
	     * [31] Busy bit
	     * [30:29] Access mode
	     * [28:24] reserved
	     * [23:15] 2.7-3.6
	     * [14:8]  2.0-2.6
	     * [7]     1.7-1.95
	     * [6:0]   reserved
	     */ 
	    
	
	
	
	
	
		
		
		
	
	    /* Now it's time to examine response */ 
	    mmc_resp = cmd.resp[0];
	
	    /* Response contains card's ocr. Update card's information */ 
	    card->ocr = mmc_resp;
	
	    /* Check the response for busy status */ 
	    if (!(mmc_resp & MMC_BOOT_OCR_BUSY))
		
		
		
	
		
		
		
	
	else
		
		
		
	



/*
 * Request any card to send its uniquie card identification (CID) number (CMD2).
 */ 
static unsigned int mmc_boot_all_send_cid(struct mmc_boot_card *card) 
{
	
	
	
	    /* basic check */ 
	    if (card == NULL)
		
		
		
	
		  
	
	    /* CMD2 Format:
	     * [31:0] stuff bits
	     */ 
	    cmd.cmd_index = CMD2_ALL_SEND_CID;
	
	
	
	
	    /* send command */ 
	    mmc_ret = mmc_boot_send_command(&cmd);
	
		
		
		
	
	    /* Response contains card's 128 bits CID register */ 
	    mmc_ret = mmc_boot_decode_and_save_cid(card, cmd.resp);
	
		
		
		
	



/*
 * Ask any card to send it's relative card address (RCA).This RCA number is
 * shorter than CID and is used by the host to address the card in future (CMD3)
 */ 
static unsigned int mmc_boot_send_relative_address(struct mmc_boot_card *card) 
{
	
	
	
	    /* basic check */ 
	    if (card == NULL)
		
		
		
	
		  
	
	    /* CMD3 Format:
	     * [31:0] stuff bits
	     */ 
	    if (card->type == MMC_BOOT_TYPE_SDHC
		|| card->type == MMC_BOOT_TYPE_STD_SD)
		
		
		
		
		
		
		    /* send command */ 
		    mmc_ret = mmc_boot_send_command(&cmd);
		
			
			
			
		
		    /* For sD, card will send RCA. Store it */ 
		    card->rca = (cmd.resp[0] >> 16);
		
	
	else
		
		
		
		
		
		
		
		    /* send command */ 
		    mmc_ret = mmc_boot_send_command(&cmd);
		
			
			
			
		
	



/*
 * Requests card to send it's CSD register's contents. (CMD9)
 */ 
static unsigned int
mmc_boot_send_csd(struct mmc_boot_card *card, 
{
	
	
	
	
	    /* basic check */ 
	    if (card == NULL)
		
		
		
	
		  
	
	    /* CMD9 Format:
	     * [31:16] RCA
	     * [15:0] stuff bits
	     */ 
	    mmc_arg |= card->rca << 16;
	
	
	
	
	
	    /* send command */ 
	    mmc_ret = mmc_boot_send_command(&cmd);
	
		
		
		
	
	    /* response contains the card csd */ 
	    memcpy(raw_csd, cmd.resp, sizeof(cmd.resp));
	



/*
 * Selects a card by sending CMD7 to the card with its RCA.
 * If RCA field is set as 0 ( or any other address ),
 * the card will be de-selected. (CMD7)
 */ 
static unsigned int
mmc_boot_select_card(struct mmc_boot_card *card, 
{
	
	
	
	
	    /* basic check */ 
	    if (card == NULL)
		
		
		
	
		  
	
	    /* CMD7 Format:
	     * [31:16] RCA
	     * [15:0] stuff bits
	     */ 
	    mmc_arg |= rca << 16;
	
	
	
	
	    /* If we are deselecting card, we do not get response */ 
	    if (rca == card->rca && rca)
		
		
		     || card->type == MMC_BOOT_TYPE_STD_SD)
			
		
		else
			
		
	
	else
		
		
		
	
	    /* send command */ 
	    mmc_ret = mmc_boot_send_command(&cmd);
	
		
		
		
	
	    /* As of now no need to look into a response. If it's required
	     * we'll explore later on */ 
	    



/*
 * Send command to set block length.
 */ 
static unsigned int
mmc_boot_set_block_len(struct mmc_boot_card *card, 
{
	
	
	
	    /* basic check */ 
	    if (card == NULL)
		
		
		
	
		  
	
	    /* CMD16 Format:
	     * [31:0] block length
	     */ 
	    
	
	
	
	
	    /* send command */ 
	    mmc_ret = mmc_boot_send_command(&cmd);
	
		
		
		
	
	    /* If blocklength is larger than 512 bytes,
	     * the card sets BLOCK_LEN_ERROR bit. */ 
	    if (cmd.resp[0] & MMC_BOOT_R1_BLOCK_LEN_ERR)
		
		
		
	



/*
 * Requests the card to stop transmission of data.
 */ 
static unsigned int
mmc_boot_send_stop_transmission(struct mmc_boot_card *card,
				
{
	
	
	
	    /* basic check */ 
	    if (card == NULL)
		
		
		
	
		  
	
	    /* CMD12 Format:
	     * [31:0] stuff bits
	     */ 
	    
	
	
	
	
	
	
	    /* send command */ 
	    mmc_ret = mmc_boot_send_command(&cmd);
	
		
		
		
	



/*
 * Get the card's current status
 */ 
static unsigned int
mmc_boot_get_card_status(struct mmc_boot_card *card,
			 
{
	
	
	
	    /* basic check */ 
	    if (card == NULL)
		
		
		
	
		  
	
	    /* CMD13 Format:
	     * [31:16] RCA
	     * [15:0] stuff bits
	     */ 
	    cmd.cmd_index = CMD13_SEND_STATUS;
	
	
	
	
	
	    /* send command */ 
	    mmc_ret = mmc_boot_send_command(&cmd);
	
		
		
		
	
	    /* Checking ADDR_OUT_OF_RANGE error in CMD13 response */ 
	    if (IS_ADDR_OUT_OF_RANGE(cmd.resp[0]))
		
		
		
	
	



/*
 * Decode type of error caused during read and write
 */ 
static unsigned int mmc_boot_status_error(unsigned mmc_status) 
{
	
	
	    /* If DATA_CRC_FAIL bit is set to 1 then CRC error was detected by
	       card/device during the data transfer */ 
	    if (mmc_status & MMC_BOOT_MCI_STAT_DATA_CRC_FAIL)
		
		
		
	
	    /* If DATA_TIMEOUT bit is set to 1 then the data transfer time exceeded
	       the data timeout period without completing the transfer */ 
	    else if (mmc_status & MMC_BOOT_MCI_STAT_DATA_TIMEOUT)
		
		
		
	
	    /* If RX_OVERRUN bit is set to 1 then SDCC2 tried to receive data from
	       the card before empty storage for new received data was available.
	       Verify that bit FLOW_ENA in MCI_CLK is set to 1 during the data xfer. */ 
	    else if (mmc_status & MMC_BOOT_MCI_STAT_RX_OVRRUN)
		
		
		    /* Note: We've set FLOW_ENA bit in MCI_CLK to 1. so no need to verify
		       for now */ 
		    mmc_ret = MMC_BOOT_E_RX_OVRRUN;
		
	
	    /* If TX_UNDERRUN bit is set to 1 then SDCC2 tried to send data to
	       the card before new data for sending was available. Verify that bit
	       FLOW_ENA in MCI_CLK is set to 1 during the data xfer. */ 
	    else if (mmc_status & MMC_BOOT_MCI_STAT_TX_UNDRUN)
		
		
		    /* Note: We've set FLOW_ENA bit in MCI_CLK to 1.so skipping it now */ 
		    mmc_ret = MMC_BOOT_E_RX_OVRRUN;
		
	



/*
 * Send ext csd command.
 */ 
static unsigned int
mmc_boot_send_ext_cmd(struct mmc_boot_card *card, unsigned char *buf) 
{
	
	
	
	
	
	
	
	
	
	    /* basic check */ 
	    if (card == NULL)
		
		
		
	
	    /* set block len */ 
	    if ((card->type != MMC_BOOT_TYPE_MMCHC)
		&& (card->type != MMC_BOOT_TYPE_SDHC))
		
		
		
			
			
				 "Error No.%d: Failure setting block length for Card (RCA:%s)\n",
				 
			
			
		
	
	    /* Set the FLOW_ENA bit of MCI_CLK register to 1 */ 
	    mmc_reg = readl(MMC_BOOT_MCI_CLK);
	
	
	
	    /* Write data timeout period to MCI_DATA_TIMER register. */ 
	    /* Data timeout period should be in card bus clock periods */ 
	    mmc_reg = 0xFFFFFFFF;
	
	
	
	    /* Set appropriate fields and write the MCI_DATA_CTL register. */ 
	    /* Set ENABLE bit to 1 to enable the data transfer. */ 
	    mmc_reg =
	    MMC_BOOT_MCI_DATA_ENABLE | MMC_BOOT_MCI_DATA_DIR | (512 <<
								MMC_BOOT_MCI_BLKSIZE_POS);
	
	
		  
	
	    /* CMD8 */ 
	    cmd.cmd_index = CMD8_SEND_EXT_CSD;
	
	
	
	
	    /* send command */ 
	    mmc_ret = mmc_boot_send_command(&cmd);
	
		
		
		
	
	    MMC_BOOT_MCI_STAT_DATA_CRC_FAIL | \
	    \
	
	    /* Read the transfer data from SDCC2 FIFO. If Data Mover is not used
	       read the data from the MCI_FIFO register as long as RXDATA_AVLBL
	       bit of MCI_STATUS register is set to 1 and bits DATA_CRC_FAIL,
	       DATA_TIMEOUT, RX_OVERRUN of MCI_STATUS register are cleared to 0.
	       Continue the reads until the whole transfer data is received */ 
	    
	do
		
		
		
		
			
			
			
			
		
			
			
			
				
				
				
			
				
				
				    /* FIFO contains 16 32-bit data buffer on 16 sequential addresses */ 
				    *mmc_ptr = readl(MMC_BOOT_MCI_FIFO + 
						     (mmc_count %
						      MMC_BOOT_MCI_FIFO_SIZE));
				
				
				    /* increase mmc_count by word size */ 
				    mmc_count += sizeof(unsigned int);
				
			    /* quit if we have read enough of data */ 
			    if (mmc_count >= 512)
				
			
		
		else if (mmc_status & MMC_BOOT_MCI_STAT_DATA_END)
			
			
			
		
	while (1);
	



/*
 * Switch command
 */ 
static unsigned int
mmc_boot_switch_cmd(struct mmc_boot_card *card, 
		    
{
	
	
	
	    /* basic check */ 
	    if (card == NULL)
		
		
		
	
		  
	
	    /* CMD6 Format:
	     * [31:26] set to 0
	     * [25:24] access
	     * [23:16] index
	     * [15:8] value
	     * [7:3] set to 0
	     * [2:0] cmd set
	     */ 
	    cmd.cmd_index = CMD6_SWITCH_FUNC;
	
	
	
	
	
	
	
		
		
		
	



/*
 * A command to set the data bus width for card. Set width to either
 */ 
static unsigned int
mmc_boot_set_bus_width(struct mmc_boot_card *card, 
{
	
	
	
	
	
	
		
		
		
	
	    mmc_boot_switch_cmd(card, MMC_BOOT_ACCESS_WRITE,
				
	
		
		
		
	
	    /* Wait for the card to complete the switch command processing */ 
	    do
		
		
		
			
			
			
		
		
			
			
			
		
	while (MMC_BOOT_CARD_STATUS(status) == MMC_BOOT_PROG_STATE);
	
	    /* set MCI_CLK accordingly */ 
	    mmc_reg = readl(MMC_BOOT_MCI_CLK);
	
	
		
		
		
	
	else if (width == MMC_BOOT_BUS_WIDTH_4_BIT)
		
		
		
	
	else if (width == MMC_BOOT_BUS_WIDTH_8_BIT)
		
		
		
	
	
	



/*
 * A command to start data read from card. Either a single block or
 * multiple blocks can be read. Multiple blocks read will continuously
 * transfer data from card to host unless requested to stop by issuing
 * CMD12 - STOP_TRANSMISSION.
 */ 
static unsigned int
mmc_boot_send_read_command(struct mmc_boot_card *card,
			   
{
	
	
	
	    /* basic check */ 
	    if (card == NULL)
		
		
		
	
		  
	
	    /* CMD17/18 Format:
	     * [31:0] Data Address
	     */ 
	    if (xfer_type == MMC_BOOT_XFER_MULTI_BLOCK)
		
		
		
	
	else
		
		
		
	
	
	
	
	    /* send command */ 
	    mmc_ret = mmc_boot_send_command(&cmd);
	
		
		
		
	
	    /* Response contains 32 bit Card status. Here we'll check
	       BLOCK_LEN_ERROR and ADDRESS_ERROR */ 
	    if (cmd.resp[0] & MMC_BOOT_R1_BLOCK_LEN_ERR)
		
		
		
	
	    /* Misaligned address not matching block length */ 
	    if (cmd.resp[0] & MMC_BOOT_R1_ADDR_ERR)
		
		
		
	



/*
 * A command to start data write to card. Either a single block or
 * multiple blocks can be written. Multiple block write will continuously
 * transfer data from host to card unless requested to stop by issuing
 * CMD12 - STOP_TRANSMISSION.
 */ 
static unsigned int
mmc_boot_send_write_command(struct mmc_boot_card *card,
			    
{
	
	
	
	    /* basic check */ 
	    if (card == NULL)
		
		
		
	
		  
	
	    /* CMD24/25 Format:
	     * [31:0] Data Address
	     */ 
	    if (xfer_type == MMC_BOOT_XFER_MULTI_BLOCK)
		
		
		
	
	else
		
		
		
	
	
	
	
	    /* send command */ 
	    mmc_ret = mmc_boot_send_command(&cmd);
	
		
		
		
	
	    /* Response contains 32 bit Card status. Here we'll check
	       BLOCK_LEN_ERROR and ADDRESS_ERROR */ 
	    if (cmd.resp[0] & MMC_BOOT_R1_BLOCK_LEN_ERR)
		
		
		
	
	    /* Misaligned address not matching block length */ 
	    if (cmd.resp[0] & MMC_BOOT_R1_ADDR_ERR)
		
		
		
	



/*
 * Write data_len data to address specified by data_addr. data_len is
 * multiple of blocks for block data transfer.
 */ 
static unsigned int
mmc_boot_write_to_card(struct mmc_boot_host *host,
		       
		       
		       
{
	
	
	
	
	
	
	
	
	
	
		
		
		
	
	    /* Set block length. High Capacity MMC/SD card uses fixed 512 bytes block
	       length. So no need to send CMD16. */ 
	    if ((card->type != MMC_BOOT_TYPE_MMCHC)
		&& (card->type != MMC_BOOT_TYPE_SDHC))
		
		
		
			
			
                    (RCA:%s)\n", mmc_ret,
				 (char *)(card->rca));
			
			
		
	
	    /* use multi-block mode to transfer for data larger than a block */ 
	    xfer_type =
	    (data_len >
	     card->rd_block_len) ? MMC_BOOT_XFER_MULTI_BLOCK :
	    
	
	    /* For MMCHC/SDHC data address is specified in unit of 512B */ 
	    addr = ((card->type != MMC_BOOT_TYPE_MMCHC)
		    && (card->type !=
			MMC_BOOT_TYPE_SDHC)) 
	    : (unsigned int)
	    (data_addr / 512);
	
	    /* Set the FLOW_ENA bit of MCI_CLK register to 1 */ 
	    mmc_reg = readl(MMC_BOOT_MCI_CLK);
	
	
	
	    /* Write data timeout period to MCI_DATA_TIMER register */ 
	    /* Data timeout period should be in card bus clock periods */ 
	    /*TODO: Fix timeout value */ 
	    mmc_reg = 0xFFFFFFFF;
	
	
	    /* Write the total size of the transfer data to MCI_DATA_LENGTH register */ 
	    writel(data_len, MMC_BOOT_MCI_DATA_LENGTH);
	
	    /* Send command to the card/device in order to start the write data xfer.
	       The possible commands are CMD24/25/53/60/61 */ 
	    mmc_ret = mmc_boot_send_write_command(card, xfer_type, addr);
	
		
		
                Card(RCA:%x)\n", mmc_ret,
			 card->rca);
		
		
	
	    /* Set appropriate fields and write the MCI_DATA_CTL register */ 
	    /* Set ENABLE bit to 1 to enable the data transfer. */ 
	    mmc_reg = 0;
	
	
	    /* Clear DIRECTION bit to 0 to enable transfer from host to card */ 
	    /* Clear MODE bit to 0 to enable block oriented data transfer. For
	       MMC cards only, if stream data transfer mode is desired, set
	       MODE bit to 1. */ 
	    /* Set DM_ENABLE bit to 1 in order to enable DMA, otherwise set 0 */ 
	    /* Write size of block to be used during the data transfer to
	       BLOCKSIZE field */ 
	    mmc_reg |= card->wr_block_len << MMC_BOOT_MCI_BLKSIZE_POS;
	
	
	    MMC_BOOT_MCI_STAT_DATA_CRC_FAIL | \
	    \
	
	    /* Write the transfer data to SDCC3 FIFO */ 
	    /* If Data Mover is used for data transfer, prepare a command list entry
	       and enable the Data Mover to work with SDCC2 */ 
	    /* If Data Mover is NOT used for data xfer: */ 
	    do
		
		
		
		
			
			
			
			
		
		    /* Write the data in MCI_FIFO register as long as TXFIFO_FULL bit of
		       MCI_STATUS register is 0. Continue the writes until the whole
		       transfer data is written. */ 
		    if (((data_len - mmc_count) >= MMC_BOOT_MCI_FIFO_SIZE / 2)
			&& 
			
			
				
				
				    /* FIFO contains 16 32-bit data buffer on 16 sequential addresses */ 
				    writel(*mmc_ptr, MMC_BOOT_MCI_FIFO + 
					   (mmc_count %
					    MMC_BOOT_MCI_FIFO_SIZE));
				
				
				    /* increase mmc_count by word size */ 
				    mmc_count += sizeof(unsigned int);
			
		
		else if (!(mmc_status & MMC_BOOT_MCI_STAT_TX_FIFO_FULL)
			 && (mmc_count != data_len))
			
			
			    /* FIFO contains 16 32-bit data buffer on 16 sequential addresses */ 
			    writel(*mmc_ptr, MMC_BOOT_MCI_FIFO + 
				   (mmc_count % MMC_BOOT_MCI_FIFO_SIZE));
			
			
			    /* increase mmc_count by word size */ 
			    mmc_count += sizeof(unsigned int);
			
		
		else if ((mmc_status & MMC_BOOT_MCI_STAT_DATA_END))
			
			
			}
		
	while (1);
	
		
		
                Card(RCA:%x)\n", mmc_ret,
			 card->rca);
		
		    /* In case of any failure happening for multi block transfer */ 
		    if (xfer_type == MMC_BOOT_XFER_MULTI_BLOCK)
			
		
		
	
	    /* Send command to the card/device in order to poll the de-assertion of
	       card/device BUSY condition. It is important to set PROG_ENA bit in
	       MCI_CLK register before sending the command. Possible commands are
	       CMD12/13. */ 
	    if (xfer_type == MMC_BOOT_XFER_MULTI_BLOCK)
		
		
		
			
			
                    command to the Card(RCA:%x)\n", mmc_ret,
				 card->rca);
			
			
		
	
	else
		
		
		
			
			
				 "Error No.%d: Failure getting card status of Card(RCA:%x)\n",
				 
			
			
		
	
	    /* Wait for interrupt or poll on PROG_DONE bit of MCI_STATUS register. If
	       PROG_DONE bit is set to 1 it means that the card finished it programming
	       and stopped driving DAT0 line to 0 */ 
	    do
		
		
		
			
			
			
		
	while (1);
	



/*
 * Adjust the interface speed to optimal speed
 */ 
static unsigned int
mmc_boot_adjust_interface_speed(struct mmc_boot_host *host,
				
{
	
	
	
	
	    /* Setting HS_TIMING in EXT_CSD (CMD6) */ 
	    mmc_ret =
	    mmc_boot_switch_cmd(card, MMC_BOOT_ACCESS_WRITE,
				
	
		
		
		
	
	    /* Wait for the card to complete the switch command processing */ 
	    do
		
		
		
			
			
			
		
		
			
			
			
		
	while (MMC_BOOT_CARD_STATUS(status) == MMC_BOOT_PROG_STATE);
	
#ifdef PLATFORM_MSM8X60
	    mmc_ret = mmc_boot_enable_clock(host, MMC_CLK_48MHZ);
	
#else				/* 
	    mmc_ret = mmc_boot_enable_clock(host, MMC_CLK_50MHZ);
	
#endif				/* 
	    if (mmc_ret != MMC_BOOT_E_SUCCESS)
		
		
		
	



mmc_boot_set_block_count(struct mmc_boot_card *card,
			 
{
	
	
	
	    /* basic check */ 
	    if (card == NULL)
		
		
		
	
		  
	
	    /* CMD23 Format:
	     * [15:0] number of blocks
	     */ 
	    
	
	
	
	
	    /* send command */ 
	    mmc_ret = mmc_boot_send_command(&cmd);
	
		
		
		
	
		
		
		
	



/*
 * Reads a data of data_len from the address specified. data_len
 * should be multiple of block size for block data transfer.
 */ 
static unsigned int
mmc_boot_read_from_card(struct mmc_boot_host *host,
			
			
			
{
	
	
	
	
	
	
	
	
	
	
		
		
		
	
	    /* Set block length. High Capacity MMC/SD card uses fixed 512 bytes block
	       length. So no need to send CMD16. */ 
	    if ((card->type != MMC_BOOT_TYPE_MMCHC)
		&& (card->type != MMC_BOOT_TYPE_SDHC))
		
		
		
			
			
				 "Error No.%d: Failure setting block length for Card (RCA:%s)\n",
				 
			
			
		
	
	    /* use multi-block mode to transfer for data larger than a block */ 
	    xfer_type =
	    (data_len >
	     card->rd_block_len) ? MMC_BOOT_XFER_MULTI_BLOCK :
	    
	
		
		
		     || (card->type == MMC_BOOT_TYPE_STD_MMC))
			
			
			    /* Virtio model does not support open-ended multi-block reads.
			     * So, block count must be set before sending read command.
			     * All SD cards do not support this command. Restrict this to MMC.
			     */ 
			    mmc_ret =
			    mmc_boot_set_block_count(card,
						     data_len /
						     (card->rd_block_len));
			
				
				
					 "Error No.%d: Failure setting read block count for Card (RCA:%s)\n",
					 
				
				
			
			
		
	
	    /* Set the FLOW_ENA bit of MCI_CLK register to 1 */ 
	    /* Note: It's already enabled */ 
	    
	    /* If Data Mover is used for data transfer then prepare Command
	       List Entry and enable the Data mover to work with SDCC2 */ 
	    /* Note: Data Mover not used */ 
	    
	    /* Write data timeout period to MCI_DATA_TIMER register. */ 
	    /* Data timeout period should be in card bus clock periods */ 
	    mmc_reg = (unsigned long)(card->rd_timeout_ns / 1000000) * 
	    (host->mclk_rate / 1000);
	
	mmc_reg = mmc_reg / 2;
	
	
	    /* Write the total size of the transfer data to MCI_DATA_LENGTH
	       register. For block xfer it must be multiple of the block
	       size. */ 
	    writel(data_len, MMC_BOOT_MCI_DATA_LENGTH);
	
	    /* For MMCHC/SDHC data address is specified in unit of 512B */ 
	    addr = ((card->type != MMC_BOOT_TYPE_MMCHC)
		    && (card->type !=
			MMC_BOOT_TYPE_SDHC)) 
	    : (unsigned int)
	    (data_addr / 512);
	
	    /* Set appropriate fields and write the MCI_DATA_CTL register. */ 
	    /* Set ENABLE bit to 1 to enable the data transfer. */ 
	    mmc_reg = 0;
	
	
	    /* Clear DIRECTION bit to 1 to enable transfer from card to host */ 
	    mmc_reg |= MMC_BOOT_MCI_DATA_DIR;
	
	    /* Clear MODE bit to 0 to enable block oriented data transfer. For
	       MMC cards only, if stream data transfer mode is desired, set
	       MODE bit to 1. */ 
	    /* Set DM_ENABLE bit to 1 in order to enable DMA, otherwise set 0 */ 
	    /* Write size of block to be used during the data transfer to
	       BLOCKSIZE field */ 
	    mmc_reg |= (card->rd_block_len << MMC_BOOT_MCI_BLKSIZE_POS);
	
	
	    /* Send command to the card/device in order to start the read data
	       transfer. Possible commands: CMD17/18/53/60/61. */ 
	    mmc_ret = mmc_boot_send_read_command(card, xfer_type, addr);
	
		
		
			 "Error No.%d: Failure sending read command to the Card(RCA:%x)\n",
			 
		
		
	
	    MMC_BOOT_MCI_STAT_DATA_CRC_FAIL | \
	    \
	
	    /* Read the transfer data from SDCC2 FIFO. If Data Mover is not used
	       read the data from the MCI_FIFO register as long as RXDATA_AVLBL
	       bit of MCI_STATUS register is set to 1 and bits DATA_CRC_FAIL,
	       DATA_TIMEOUT, RX_OVERRUN of MCI_STATUS register are cleared to 0.
	       Continue the reads until the whole transfer data is received */ 
	    
	do
		
		
		
		
			
			
			
			
		
			
			
			
				
				
				
			
				
				
				    /* FIFO contains 16 32-bit data buffer on 16 sequential addresses */ 
				    *mmc_ptr = readl(MMC_BOOT_MCI_FIFO + 
						     (mmc_count %
						      MMC_BOOT_MCI_FIFO_SIZE));
				
				
				    /* increase mmc_count by word size */ 
				    mmc_count += sizeof(unsigned int);
				
			    /* quit if we have read enough of data */ 
			    if (mmc_count == data_len)
				
			
		
		else if (mmc_status & MMC_BOOT_MCI_STAT_DATA_END)
			
			
			
		
	while (1);
	
		
		
                Card(RCA:%x)\n", mmc_ret,
			 card->rca);
		
		
	
	    /* In case a multiple block transfer was performed, send CMD12 to the
	       card/device in order to indicate the end of read data transfer */ 
	    if ((xfer_type == MMC_BOOT_XFER_MULTI_BLOCK) && open_ended_read)
		
		
		
			
			
                    command to the Card(RCA:%x)\n", mmc_ret,
				 card->rca);
			
			
		
	



/*
 * Initialize host structure, set and enable clock-rate and power mode.
 */ 
unsigned int mmc_boot_init(struct mmc_boot_host *host) 
{
	
	
	
	
	
	
	    /* clock frequency should be less than 400KHz in identification mode */ 
	    mmc_ret = mmc_boot_enable_clock(host, MMC_CLK_400KHZ);
	
		
		
		
	
	    /* set power mode */ 
	    /* give some time to reach minimum voltate */ 
	    mdelay(2);
	
	
	
	
	
	    /* some more time to stabilize voltage */ 
	    mdelay(2);
	



/*
 * Performs card identification process:
 * - get card's unique identification number (CID)
 * - get(for sd)/set (for mmc) relative card address (RCA)
 * - get CSD
 * - select the card, thus transitioning it to Transfer State
 * - get Extended CSD (for mmc)
 */ 
static unsigned int
mmc_boot_identify_card(struct mmc_boot_host *host,
		       
{
	
	
	
	    /* basic check */ 
	    if ((host == NULL) || (card == NULL))
		
		
		
	
	    /* Ask card to send its unique card identification (CID) number (CMD2) */ 
	    mmc_return = mmc_boot_all_send_cid(card);
	
		
		
			 "Error No. %d: Failure getting card's CID number!\n",
			 
		
		
	
	    /* Ask card to send a relative card address (RCA) (CMD3) */ 
	    mmc_return = mmc_boot_send_relative_address(card);
	
		
		
			 "Error No. %d: Failure getting card's RCA!\n",
			 
		
		
	
	    /* Get card's CSD register (CMD9) */ 
	    mmc_return = mmc_boot_send_csd(card, raw_csd);
	
		
		
			 "Error No.%d: Failure getting card's CSD information!\n",
			 
		
		
	
	    /* Select the card (CMD7) */ 
	    mmc_return = mmc_boot_select_card(card, card->rca);
	
		
		
			 "Error No.%d: Failure selecting the Card with RCA: %x\n",
			 
		
		
	
	    /* Set the card status as active */ 
	    card->status = MMC_BOOT_STATUS_ACTIVE;
	
	       || (card->type == MMC_BOOT_TYPE_MMCHC))
		
		
		    /* For MMC cards, also get the extended csd */ 
		    mmc_return = mmc_boot_send_ext_cmd(card, ext_csd_buf);
		
			
			
				 "Error No.%d: Failure getting card's ExtCSD information!\n",
				 
			
			
		
	
	    /* Decode and save the CSD register */ 
	    mmc_return = mmc_boot_decode_and_save_csd(card, raw_csd);
	
		
		
			 "Error No.%d: Failure decoding card's CSD information!\n",
			 
		
		
	
	    /* Once CSD is received, set read and write timeout value now itself */ 
	    mmc_return = mmc_boot_set_read_timeout(host, card);
	
		
		
			 "Error No.%d: Failure setting Read Timeout value!\n",
			 
		
		
	
	
		
		
			 "Error No.%d: Failure setting Write Timeout value!\n",
			 
		
		
	



{
	
	
	
		  
	
	
	
	
	
	
		
		
		
	



{
	
	
	
	
		  
	
	    /* Send CMD8 to set interface condition */ 
	    for (i = 0; i < 3; i++)
		
		
		
		
		
		
		
			
			
				
			
			    /* Set argument for ACMD41 */ 
			    ocr_cmd_arg =
			    MMC_BOOT_SD_NEG_OCR | MMC_BOOT_SD_HC_HCS;
			
			
		
		
	
	    /* Send ACMD41 to set operating condition */ 
	    /* Try for a max of 1 sec as per spec */ 
	    for (i = 0; i < 20; i++)
		
		
		
			
			
			
		
		
		
		
		
		
			
			
			
		
		else if (cmd.resp[0] & MMC_BOOT_SD_DEV_READY)
			
			
			    /* Check for HC */ 
			    if (cmd.resp[0] & (1 << 30))
				
				
				
			
			else
				
				
				
			
			
		
		
	



/*
 * Routine to initialize MMC card. It resets a card to idle state, verify operating
 * voltage and set the card inready state.
 */ 
static unsigned int
mmc_boot_init_card(struct mmc_boot_host *host, 
{
	
	
	
	    /* basic check */ 
	    if ((host == NULL) || (card == NULL))
		
		
		
	
	    /* 1. Card Reset - CMD0 */ 
	    mmc_return = mmc_boot_reset_cards();
	
		
		
			 "Error No.:%d: Failure resetting MMC cards!\n",
			 mmc_return);
		
		
	
	    /* 2. Card Initialization process */ 
	    
	    /* Send CMD1 to identify and reject cards that do not match host's VDD range
	       profile. Cards sends its OCR register in response.
	     */ 
	    mmc_retry = 0;
	
	do
		
		
		
		    /* Card returns busy status. We'll retry again! */ 
		    if (mmc_return == MMC_BOOT_E_CARD_BUSY)
			
			
			
			
			
		
		else if (mmc_return == MMC_BOOT_E_SUCCESS)
			
			
			
		
		else
			
			
				 "Error No. %d: Failure Initializing MMC Card!\n",
				 
			
			    /* Check for sD card */ 
			    mmc_return = mmc_boot_sd_init_card(card);
			
			
		
	while (mmc_retry < host->cmd_retry);
	
	    /* If card still returned busy status we are out of luck.
	     * Card cannot be initialized */ 
	    if (mmc_return == MMC_BOOT_E_CARD_BUSY)
		
		
                Initialization not completed\n", mmc_return);
		
		
	



mmc_boot_set_sd_bus_width(struct mmc_boot_card *card, unsigned int width) 
{
	
	
	
	
	
		
		
		
	
		  
	
	    /* Send ACMD6 to set bus width */ 
	    cmd.cmd_index = ACMD6_SET_BUS_WIDTH;
	
	    /* 10 => 4 bit wide */ 
	    cmd.argument = (1 << 1);
	
	
	
	
		
		
		
	
	    /* set MCI_CLK accordingly */ 
	    sd_reg = readl(MMC_BOOT_MCI_CLK);
	
	
		
		
		
	
	else if (width == MMC_BOOT_BUS_WIDTH_4_BIT)
		
		
		
	
	else if (width == MMC_BOOT_BUS_WIDTH_8_BIT)
		
		
		
	
	
	



mmc_boot_set_sd_hs(struct mmc_boot_host *host, struct mmc_boot_card *card) 
{
	
	
	
		  
	
	    /* Send CMD6 function mode 1 to set high speed */ 
	    /* Not using mode 0 to read current consumption */ 
	    cmd.cmd_index = CMD6_SWITCH_FUNC;
	
	
	
	
	
		
		
		
	
	
#ifdef PLATFORM_MSM8X60
	    mmc_ret = mmc_boot_enable_clock(host, MMC_CLK_48MHZ);
	
#else				/* 
	    mmc_ret = mmc_boot_enable_clock(host, MMC_CLK_50MHZ);
	
#endif				/* 
	    if (mmc_ret != MMC_BOOT_E_SUCCESS)
		
		
		
	



/*
 * Performs initialization and identification of all the MMC cards connected
 * to the host.
 */ 

mmc_boot_init_and_identify_cards(struct mmc_boot_host *host,
				 struct mmc_boot_card *card) 
{
	
	
	
	    /* Basic check */ 
	    if (host == NULL)
		
		
		
	
	    /* Initialize MMC card structure */ 
	    card->status = MMC_BOOT_STATUS_INACTIVE;
	
	
	
	    /* Start initialization process (CMD0 & CMD1) */ 
	    mmc_return = mmc_boot_init_card(host, card);
	
		
		
		
	
	    /* Identify (CMD2, CMD3 & CMD9) and select the card (CMD7) */ 
	    mmc_return = mmc_boot_identify_card(host, card);
	
		
		
		
	
	       || card->type == MMC_BOOT_TYPE_STD_SD)
		
		
		
			
			
			
		
		    mmc_boot_set_sd_bus_width(card, MMC_BOOT_BUS_WIDTH_4_BIT);
		
			
			
			
		
	
	else
		
		
		    /* set interface speed */ 
		    mmc_return = mmc_boot_adjust_interface_speed(host, card);
		
			
			
				 "Error No.%d: Error adjusting interface speed!\n",
				 
			
			
		
		    /* enable wide bus */ 
		    mmc_return =
		    mmc_boot_set_bus_width(card, MMC_BOOT_BUS_WIDTH_4_BIT);
		
			
			
				 "Error No.%d: Failure to set wide bus for Card(RCA:%x)\n",
				 
			
			
		
	
	    /* Just checking whether we're in TRAN state after changing speed and bus width */ 
	    mmc_return = mmc_boot_get_card_status(card, 0, &status);
	
		
		
		
	
		
	



/*
 * Read MBR from MMC card and fill partition table.
 */ 
static unsigned int mmc_boot_read_MBR(void) 
{
	
	
	
	
	
	
	
	
	    /* Print out the MBR first */ 
	    ret =
	    mmc_boot_read_from_card(&mmc_host, &mmc_card, 0,
				    \
				    \
	
		
		
		
	
	    /* Check to see if signature exists */ 
	    if ((buffer[TABLE_SIGNATURE] != 0x55) || \
		(buffer[TABLE_SIGNATURE + 1] != 0xAA))
		
		
		
		
	
	    /* Print out the first 4 partition */ 
	    idx = TABLE_ENTRY_0;
	
		
		
		    \
		
		    \
		
		    \
					  [idx + \
					   \
		
		    \
					  [idx + \
					   \
		
		
		
			       \
		
		
			
		
	
	    /* See if the last partition is EBR, if not, parsing is done */ 
	    if (dtype != 0x05)
		
		
		
	
	
	
					 (EBR_first_sec * 512),
					 \
					 \
	
		
		
		
	
	    /* Loop to parse the EBR */ 
	    for (i = 0;; i++)
		
		
		     || (buffer[TABLE_SIGNATURE + 1] != 0xAA))
			
			
			
		
		    \
		
		    \
		
		    \
					  [TABLE_ENTRY_0 +
					   \
		    \
		
		    \
					  [TABLE_ENTRY_0 + \
		
			       \
		
		
			
		
		    GET_LWORD_FROM_BYTE(&buffer
					[TABLE_ENTRY_1 + OFFSET_FIRST_SEC]);
		
			
			
			    /* Getting to the end of the EBR tables */ 
			    break;
			
		
		    /* More EBR to follow - read in the next EBR sector */ 
		    ret = mmc_boot_read_from_card(&mmc_host, &mmc_card, \
						  ((EBR_first_sec +
						    dfirstsec) * 512),
						  \
						  \
		
			
			
			
		
		
	



/*
 * Entry point to MMC boot process
 */ 
unsigned int mmc_boot_main(unsigned char slot, unsigned int base) 
{
	
	
		  sizeof(struct mmc_boot_host));
	
		sizeof(struct mmc_boot_card));
	
	
	
#ifndef PLATFORM_MSM8X60
	    /* Waiting for modem to come up */ 
	    while (readl(MSM_SHARED_BASE + 0x14) != 1) ;
	
#endif				/* 
	    /* Initialize necessary data structure and enable/set clock and power */ 
	    dprintf(INFO, " Initializing MMC host data structure and clock!\n");
	
	
		
		
			 "MMC Boot: Error Initializing MMC Card!!!\n");
		
		
	
	    /* Initialize and identify cards connected to host */ 
	    mmc_ret = mmc_boot_init_and_identify_cards(&mmc_host, &mmc_card);
	
		
		
		
		
	
	
	
	    /* Read MBR of the card */ 
	    mmc_ret = mmc_boot_read_MBR();
	
		
		
		
		
	



/*
 * MMC write function
 */ 
unsigned int
mmc_write(unsigned long long data_addr, unsigned int data_len,
	  unsigned int *in) 
{
	
	
	
	
	
		
	
		
		
		    mmc_boot_write_to_card(&mmc_host, &mmc_card,
					   \
					   sptr);
		
			
			
			
		
		
		
	
		
		
		    mmc_boot_write_to_card(&mmc_host, &mmc_card,
					   \
					   sptr);
		
	



/*
 * MMC read function
 */ 
unsigned int
mmc_read(unsigned long long data_addr, unsigned int *out,
	 unsigned int data_len) 
{
	
	
	    mmc_boot_read_from_card(&mmc_host, &mmc_card, data_addr, data_len,
				    out);
	



/*
 * Fill name for android partition found.
 */ 
static void mbr_fill_name(struct mbr_entry *mbr_ent, unsigned int type) 
{
	
		
		
	
	
		
		    /* if already assigned last name available then return */ 
		    if (!strcmp
			((const char *)vfat_partitions[vfat_count], "NONE"))
			
		
			(const char *)vfat_partitions[vfat_count]);
		
		
	
		
		
	
		
		
	
		
		
	
		
		
	
		
		
	
		
		
	
		
		
	
		
		
	
		
		
	
		
		
	
		
			(const char *)ext3_partitions[ext3_count]);
		
		
	
		
		
		



/*
 * Returns offset of given partition
 */ 
unsigned long long mmc_ptn_offset(unsigned char *name) 
{
	
	
		
			
		
	
	



{
	
	
		
			
		
	
	



/*
 * Function to read registers from MMC or SD card
 */ 
static unsigned int
mmc_boot_read_reg(struct mmc_boot_card *card, 
		  
		  
{
	
	
	
	
	
	
	
	
	
	    /* Set the FLOW_ENA bit of MCI_CLK register to 1 */ 
	    mmc_reg = readl(MMC_BOOT_MCI_CLK);
	
	
	
	    /* Write data timeout period to MCI_DATA_TIMER register. */ 
	    /* Data timeout period should be in card bus clock periods */ 
	    mmc_reg = 0xFFFFFFFF;
	
	
	
	    /* Set appropriate fields and write the MCI_DATA_CTL register. */ 
	    /* Set ENABLE bit to 1 to enable the data transfer. */ 
	    mmc_reg =
	    MMC_BOOT_MCI_DATA_ENABLE | MMC_BOOT_MCI_DATA_DIR | (data_len <<
								MMC_BOOT_MCI_BLKSIZE_POS);
	
	
		  
	
	
	
	
	
	    /* send command */ 
	    mmc_ret = mmc_boot_send_command(&cmd);
	
		
		
		
	
	    MMC_BOOT_MCI_STAT_DATA_CRC_FAIL | \
	    \
	
	do
		
		
		
		
			
			
			
			
		
			
			
			
				
				
				
			
				
				
				    /* FIFO contains 16 32-bit data buffer on 16 sequential addresses */ 
				    *mmc_ptr = readl(MMC_BOOT_MCI_FIFO + 
						     (mmc_count %
						      MMC_BOOT_MCI_FIFO_SIZE));
				
				
				    /* increase mmc_count by word size */ 
				    mmc_count += sizeof(unsigned int);
				
			    /* quit if we have read enough of data */ 
			    if (mmc_count == data_len)
				
			
		
		else if (mmc_status & MMC_BOOT_MCI_STAT_DATA_END)
			
			
			
		
	while (1);
	
		
		
                Card(RCA:%x)\n", mmc_ret,
			 card->rca);
		
		
	



/*
 * Function to set/clear power-on write protection for the user area partitions
 */ 
static unsigned int
mmc_boot_set_clr_power_on_wp_user(struct mmc_boot_card *card,
				  
				  
{
	
	
	
	
	
		  
	
	    /* Disabling PERM_WP for USER AREA (CMD6) */ 
	    mmc_ret =
	    mmc_boot_switch_cmd(card, MMC_BOOT_ACCESS_WRITE,
				
				
	
		
		
		
	
	    /* Sending CMD13 to check card status */ 
	    do
		
		
		
			
		
	while ((mmc_ret == MMC_BOOT_E_SUCCESS) && 
	       (MMC_BOOT_CARD_STATUS(status) == MMC_BOOT_PROG_STATE));
	
		
		
		
	
	
		
		
		
	
	    /* Make sure power-on write protection for user area is not disabled
	       and permanent write protection for user area is not enabled */ 
	    
		  (MMC_BOOT_EXT_USER_WP, MMC_BOOT_US_PERM_WP_EN))
		 ||
		 
		   (MMC_BOOT_EXT_USER_WP, MMC_BOOT_US_PWR_WP_DIS)))
		
		
		
	
		
		
		    /* wp_group_size = 512KB * HC_WP_GRP_SIZE * HC_ERASE_GRP_SIZE.
		       Getting write protect group size in sectors here. */ 
		    
		    (512 * 1024) * ext_csd_buf[MMC_BOOT_EXT_HC_WP_GRP_SIZE] *
		    
		    
		
	
	else
		
		
		    /* wp_group_size = (WP_GRP_SIZE + 1) * (ERASE_GRP_SIZE + 1)
		     * (ERASE_GRP_MULT + 1).
		     This is defined as the number of write blocks directly */ 
		    
		    (card->csd.erase_grp_mult + 1) * 
						       1);
		
	
		
		
		
	
	    /* Setting POWER_ON_WP for USER AREA (CMD6) */ 
	    
	    mmc_boot_switch_cmd(card, MMC_BOOT_ACCESS_WRITE,
				
	
		
		
		
	
	    /* Sending CMD13 to check card status */ 
	    do
		
		
		
			
		
	while ((mmc_ret == MMC_BOOT_E_SUCCESS) && 
	       (MMC_BOOT_CARD_STATUS(status) == MMC_BOOT_PROG_STATE));
	
		
		
		
	
	    /* Calculating the loop count for sending SET_WRITE_PROTECT (CMD28)
	       or CLEAR_WRITE_PROTECT (CMD29).
	       We are write protecting the partitions in blocks of write protect
	       group sizes only */ 
	    
		
		
		
	
	else
		
		
		
	
		
	
	else
		
	
	
	
		
		
		    /* Sending CMD28 for each WP group size
		       address is in sectors already */ 
		    cmd.argument = (addr + (i * wp_group_size));
		
		
			
			
			
		
		    /* Checking ADDR_OUT_OF_RANGE error in CMD28 response */ 
		    if (IS_ADDR_OUT_OF_RANGE(cmd.resp[0]))
			
			
			
		
		    /* Sending CMD13 to check card status */ 
		    do
			
			
			
			     MMC_BOOT_TRAN_STATE)
				
			
		while ((mmc_ret == MMC_BOOT_E_SUCCESS) && 
		       (MMC_BOOT_CARD_STATUS(status) == MMC_BOOT_PROG_STATE));
		
			
			
			
		
	



/*
 * Function to get Write Protect status of the given sector
 */ 
static unsigned int
mmc_boot_get_wp_status(struct mmc_boot_card *card, 
{
	
	
	
	    mmc_boot_read_reg(card, 8, CMD31_SEND_WRITE_PROT_TYPE, sector,
			      wp_status_buf);
	



/*
 * Test Function for setting Write protect for given sector
 */ 
static unsigned int
mmc_wp(unsigned int sector, unsigned int size, 
{
	
	
	    /* Checking whether group write protection feature is available */ 
	    if (mmc_card.csd.wp_grp_enable)
		
		
		
		    mmc_boot_set_clr_power_on_wp_user(&mmc_card, sector, size,
						      set_clear_wp);
		
		
		
	
	else
		



{
	
	


 mmc_display_ext_csd(void) 
{
	
	
	


 mmc_display_csd(void) 
{
	
	
	
	
	
	

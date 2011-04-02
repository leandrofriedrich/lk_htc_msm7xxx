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
#include <platform/iomap.h>
#include <platform/irqs.h>
#include <platform/interrupts.h>
#include <platform/gpio_hw.h>
#include <dev/uart.h>
#include "uart_dm.h"
    
#ifndef NULL
#define NULL        0
#endif				/* 
    
/* Note:
 * This is a basic implementation of UART_DM protocol. More focus has been
 * given on simplicity than efficiency. Few of the things to be noted are:
 * - RX path may not be suitable for multi-threaded scenaraio because of the
 *   use of static variables. TX path shouldn't have any problem though. If
 *   multi-threaded support is required, a simple data-structure can
 *   be maintained for each thread.
 * - Right now we are using polling method than interrupt based.
 * - We are using legacy UART protocol without Data Mover.
 * - Not all interrupts and error events are handled.
 * - While waiting Watchdog hasn't been taken into consideration.
 */ 
    
#define PACK_CHARS_INTO_WORDS(a, cnt, word)  {                                 \
    word = 0;
\
	\
	\
	\

/* Static Function Prototype Declarations */ 
static unsigned int msm_boot_uart_config_gpios(void);



						      
						      
						      *num_of_chars_out);



					    



/* Extern functions */ 
void clock_config(unsigned int ns, unsigned int md, 
		  unsigned int md_addr);

			 uint8_t pull, 


/*
 * Helper function to replace Line Feed char "\n" with
 * Carriage Return "\r\n".
 * Currently keeping it simple than efficient
 */ 
static unsigned int
msm_boot_uart_replace_lr_with_cr(char *data_in, 
				 
{
	
	
		
		
		
	
		
		
			
			
			
		
		
	
	



{
	
	    /* GPIO Pin: MSM_BOOT_UART_DM_RX_GPIO (117)
	       Function: 2
	       Direction: IN
	       Pull: No PULL
	       Drive Strength: 8 ma
	       Output Enable: Disable
	     */ 
	    gpio_tlmm_config(MSM_BOOT_UART_DM_RX_GPIO, 2, GPIO_INPUT,
			     
	
	    /* GPIO Pin: MSM_BOOT_UART_DM_TX_GPIO (118)
	       Function: 2
	       Direction: OUT
	       Pull: No PULL
	       Drive Strength: 8 ma
	       Output Enable: Disable
	     */ 
	    gpio_tlmm_config(MSM_BOOT_UART_DM_TX_GPIO, 2, GPIO_OUTPUT,
			     
	



{
	
	
	    /* Vote for PLL8 to be enabled */ 
	    curr_value = readl(MSM_BOOT_PLL_ENABLE_SC0);
	
	
	
	    /* Proceed only after PLL is enabled */ 
	    while (!(readl(MSM_BOOT_PLL8_STATUS) & (1 << 16))) ;
	
	    /* PLL8 is enabled. Enable gsbi_uart_clk */ 
	    
	    /* GSBI clock frequencies for UART protocol
	     * Operating mode          gsbi_uart_clk
	     * UART up to 115.2 Kbps   1.8432 MHz
	     * UART up to 460.8 Kbps   7.3728 MHz
	     * UART up to 4 Mbit/s     64 MHz
	     *
	     
	     * Choosing lowest supported value
	     * Rate (KHz)   NS          MD
	     * 3686400      0xFD940043  0x0006FD8E
	     */ 
	    
			  
	
	    /* Enable gsbi_pclk */ 
	    writel(0x10, MSM_BOOT_UART_DM_GSBI_HCLK_CTL);
	



/*
 * Initialize and configure GSBI for operation
 */ 
static unsigned int msm_boot_uart_dm_gsbi_init(void) 
{
	
	    /* Configure the clock block */ 
#ifndef PLATFORM_MSM8960
	    msm_boot_uart_dm_config_clock();
	
#endif				/* 
	    
	    /* Configure TLMM/GPIO to provide connectivity between GSBI
	       product ports and chip pads */ 
	    msm_boot_uart_dm_config_gpios();
	
	    /* Configure Data Mover for GSBI operation.
	     * Currently not supported. */ 
	    
	    /* Configure GSBI for UART_DM protocol.
	     * I2C on 2 ports, UART (without HS flow control) on the other 2. */ 
	    writel(0x60, MSM_BOOT_GSBI_CTRL_REG);
	



/*
 * Reset the UART
 */ 
static unsigned int msm_boot_uart_dm_reset(void) 
{
	
	
	
	
	
	



/*
 * Initialize UART_DM - configure clock and required registers.
 */ 
static unsigned int msm_boot_uart_dm_init(void) 
{
	
	    /* Configure GSB12 for uart dm */ 
	    msm_boot_uart_dm_gsbi_init();
	
	    /* Configure clock selection register for tx and rx rates.
	     * Selecting 115.2k for both RX and TX */ 
	    writel(MSM_BOOT_UART_DM_RX_TX_BIT_RATE, MSM_BOOT_UART_DM_CSR);
	
	    /* Configure UART mode registers MR1 and MR2 */ 
	    /* Hardware flow control isn't supported */ 
	    writel(0x0, MSM_BOOT_UART_DM_MR1);
	
	    /* 8-N-1 configuration: 8 data bits - No parity - 1 stop bit */ 
	    writel(MSM_BOOT_UART_DM_8_N_1_MODE, MSM_BOOT_UART_DM_MR2);
	
	    /* Configure Interrupt Mask register IMR */ 
	    writel(MSM_BOOT_UART_DM_IMR_ENABLED, MSM_BOOT_UART_DM_IMR);
	
	    /* Configure Tx and Rx watermarks configuration registers */ 
	    /* TX watermark value is set to 0 - interrupt is generated when
	     * FIFO level is less than or equal to 0 */ 
	    writel(MSM_BOOT_UART_DM_TFW_VALUE, MSM_BOOT_UART_DM_TFWR);
	
	    /* RX watermark value */ 
	    writel(MSM_BOOT_UART_DM_RFW_VALUE, MSM_BOOT_UART_DM_RFWR);
	
	    /* Configure Interrupt Programming Register */ 
	    /* Set initial Stale timeout value */ 
	    writel(MSM_BOOT_UART_DM_STALE_TIMEOUT_LSB, MSM_BOOT_UART_DM_IPR);
	
	    /* Configure IRDA if required */ 
	    /* Disabling IRDA mode */ 
	    writel(0x0, MSM_BOOT_UART_DM_IRDA);
	
	    /* Configure and enable sim interface if required */ 
	    
	    /* Configure hunt character value in HCR register */ 
	    /* Keep it in reset state */ 
	    writel(0x0, MSM_BOOT_UART_DM_HCR);
	
	    /* Configure Rx FIFO base address */ 
	    /* Both TX/RX shares same SRAM and default is half-n-half.
	     * Sticking with default value now.
	     * As such RAM size is (2^RAM_ADDR_WIDTH, 32-bit entries).
	     * We have found RAM_ADDR_WIDTH = 0x7f */ 
	    
	    /* Issue soft reset command */ 
	    msm_boot_uart_dm_reset();
	
	    /* Enable/Disable Rx/Tx DM interfaces */ 
	    /* Data Mover not currently utilized. */ 
	    writel(0x0, MSM_BOOT_UART_DM_DMEN);
	
	    /* Enable transmitter and receiver */ 
	    writel(MSM_BOOT_UART_DM_CR_RX_ENABLE, MSM_BOOT_UART_DM_CR);
	
	
	    /* Initialize Receive Path */ 
	    msm_boot_uart_dm_init_rx_transfer();
	



/*
 * Initialize Receive Path
 */ 
static unsigned int msm_boot_uart_dm_init_rx_transfer(void) 
{
	
	
	
	
	



/*
 * UART Receive operation
 * Reads a word from the RX FIFO.
 */ 
static unsigned int msm_boot_uart_dm_read(unsigned int *data, int wait) 
{
	
	
	
		
		
		
	
	    /* We will be polling RXRDY status bit */ 
	    while (!(readl(MSM_BOOT_UART_DM_SR) & MSM_BOOT_UART_DM_SR_RXRDY))
		
		
		    /* if this is not a blocking call, we'll just return */ 
		    if (!wait)
			
			
			
		
	
	    /* Check for Overrun error. We'll just reset Error Status */ 
	    if (readl(MSM_BOOT_UART_DM_SR) & MSM_BOOT_UART_DM_SR_UART_OVERRUN)
		
		
			MSM_BOOT_UART_DM_CR);
		
	
	    /* RX FIFO is ready; read a word. */ 
	    *data = readl(MSM_BOOT_UART_DM_RF(0));
	
	    /* increment the total count of chars we've read so far */ 
	    rx_chars_read_since_last_xfer += 4;
	
	    /* Rx transfer ends when one of the conditions is met:
	     * - The number of characters received since the end of the previous xfer
	     *   equals the value written to DMRX at Transfer Initialization
	     * - A stale event occurred
	     */ 
	    
	    /* If RX transfer has not ended yet */ 
	    if (rx_last_snap_count == 0)
		
		
		    /* Check if we've received stale event */ 
		    if (readl(MSM_BOOT_UART_DM_MISR) & MSM_BOOT_UART_DM_RXSTALE)
			
			
			    /* Send command to reset stale interrupt */ 
			    writel(MSM_BOOT_UART_DM_CMD_RES_STALE_INT,
				   MSM_BOOT_UART_DM_CR);
			
		
		    /* Check if we haven't read more than DMRX value */ 
		    else if ((unsigned int)rx_chars_read_since_last_xfer <
			     
			
			
			    /* We can still continue reading before initializing RX transfer */ 
			    return MSM_BOOT_UART_DM_E_SUCCESS;
			
		
		    /* If we've reached here it means RX xfer end conditions been met */ 
		    
		    /* Read UART_DM_RX_TOTAL_SNAP register to know how many valid chars
		     * we've read so far since last transfer */ 
		    rx_last_snap_count = readl(MSM_BOOT_UART_DM_RX_TOTAL_SNAP);
		
	
	    /* If there are still data left in FIFO we'll read them before
	     * initializing RX Transfer again */ 
	    if ((rx_last_snap_count - rx_chars_read_since_last_xfer) >= 0)
		
		
		
	
	
	
	



/*
 * UART transmit operation
 */ 
static unsigned int
msm_boot_uart_dm_write(char *data, 
{
	
	
	
	
	
	
	
		
		
		
	
	    /* Replace line-feed (/n) with carriage-return + line-feed (/r/n) */ 
	    
	
	
	
	    /* Write to NO_CHARS_FOR_TX register number of characters
	     * to be transmitted. However, before writing TX_FIFO must
	     * be empty as indicated by TX_READY interrupt in IMR register
	     */ 
	    
	    /* Check if transmit FIFO is empty.
	     * If not we'll wait for TX_READY interrupt. */ 
	    if (!(readl(MSM_BOOT_UART_DM_SR) & MSM_BOOT_UART_DM_SR_TXEMT))
		
		
			(readl(MSM_BOOT_UART_DM_ISR) &
			 MSM_BOOT_UART_DM_TX_READY))
			
			
			
			    /* Kick watchdog? */ 
			}
		
	
	    /* We are here. FIFO is ready to be written. */ 
	    /* Write number of characters to be written */ 
	    writel(num_of_chars, MSM_BOOT_UART_DM_NO_CHARS_FOR_TX);
	
	    /* Clear TX_READY interrupt */ 
	    writel(MSM_BOOT_UART_DM_GCMD_RES_TX_RDY_INT, MSM_BOOT_UART_DM_CR);
	
	    /* We use four-character word FIFO. So we need to divide data into
	     * four characters and write in UART_DM_TF register */ 
	    tx_word_count = (num_of_chars % 4) ? ((num_of_chars / 4) + 1) : 
	    (num_of_chars / 4);
	
	
		
		
		
		
		    /* Wait till TX FIFO has space */ 
		    while (!
			   (readl(MSM_BOOT_UART_DM_SR) &
			    MSM_BOOT_UART_DM_SR_TXRDY))
			
			
			
		
		    /* TX FIFO has space. Write the chars */ 
		    writel(tx_word, MSM_BOOT_UART_DM_TF(0));
		
		
		
	



/* Defining functions that's exposed to outside world and in coformance to
 * existing uart implemention. These functions are being called to initialize
 * UART and print debug messages in bootloader. */ 

{
	
	
	


/* UART_DM uses four character word FIFO where as UART core
 * uses a character FIFO. so it's really inefficient to try
 * to write single character. But that's how dprintf has been
 * implemented.
 */ 
int uart_putc(int port, char c) 
{
	
	



/* UART_DM uses four character word FIFO whereas uart_getc
 * is supposed to read only one character. So we need to
 * read a word and keep track of each character in the word.
 */ 
int uart_getc(int port, bool wait) 
{
	
	
	
		
		
		    /* Read from FIFO only if it's a first read or all the four
		     * characters out of a word have been read */ 
		    if (msm_boot_uart_dm_read(&word, wait) !=
			MSM_BOOT_UART_DM_E_SUCCESS)
			
			
			
		
	
	
	



/*
 * Copyright (c) 2015-2016 iWave Systems Technologies Pvt. Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2
 * as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

/*
 * @file iwg20m_q7.h 
 *
 * @brief unused GPIO can be defined as input for Q7 SOMs
 *
 * @ingroup Main
 */

extern int sh_timer_init(void);
extern int sdhi_mmc_init(unsigned long addr, int ch);
extern int mmcif_mmc_init(void);
extern int ravb_initialize(bd_t *bd);

extern void arch_preboot_os(void);

extern void qos_init(void);

/*
 *  linux/drivers/video/kyro/STG4000Interface.h
 *
 *  Copyright (C) 2002 STMicroelectronics
 *
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file COPYING in the main directory of this archive
 * for more details.
 */

#ifndef _STG4000INTERFACE_H
#define _STG4000INTERFACE_H

#include <linux/pci.h>
#include <video/kyro.h>

/*
 * Ramdac Setup
 */
extern int InitialiseRamdac(STG4000REG __iomem *pSTGReg, u32 displayDepth,
			    u32 displayWidth, u32 displayHeight,
			    s32 HSyncPolarity, s32 VSyncPolarity,
			    u32 *pixelClock);

extern void DisableRamdacOutput(STG4000REG __iomem *pSTGReg);
extern void EnableRamdacOutput(STG4000REG __iomem *pSTGReg);

/*
 * Timing generator setup
 */
extern void DisableVGA(STG4000REG __iomem *pSTGReg);
extern void StopVTG(STG4000REG __iomem *pSTGReg);
extern void StartVTG(STG4000REG __iomem *pSTGReg);
extern void SetupVTG(STG4000REG __iomem *pSTGReg,
		     const struct kyrofb_info * pTiming);

extern u32 ProgramClock(u32 refClock, u32 coreClock, u32 *FOut, u32 *ROut, u32 *POut);
extern int SetCoreClockPLL(STG4000REG __iomem *pSTGReg, struct pci_dev *pDev);

/*
 * Overlay setup
 */
extern void ResetOverlayRegisters(STG4000REG __iomem *pSTGReg);

extern int CreateOverlaySurface(STG4000REG __iomem *pSTGReg,
				u32 ulWidth, u32 ulHeight,
				int bLinear,
				u32 ulOverlayOffset,
				u32 * retStride, u32 * retUVStride);

extern int SetOverlayBlendMode(STG4000REG __iomem *pSTGReg,
			       OVRL_BLEND_MODE mode,
			       u32 ulAlpha, u32 ulColorKey);

extern int SetOverlayViewPort(STG4000REG __iomem *pSTGReg,
			      u32 left, u32 top,
			      u32 right, u32 bottom);

extern void EnableOverlayPlane(STG4000REG __iomem *pSTGReg);

#endif /* _STG4000INTERFACE_H */

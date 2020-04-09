/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (c) 2015 MediaTek Inc.
 */

#ifndef __MTK_MMSYS_H
#define __MTK_MMSYS_H

enum mtk_ddp_comp_id;
struct device;

#if IS_ENABLED(CONFIG_MTK_MMSYS)
void mtk_mmsys_ddp_connect(struct device *dev,
			   enum mtk_ddp_comp_id cur,
			   enum mtk_ddp_comp_id next);

void mtk_mmsys_ddp_disconnect(struct device *dev,
			      enum mtk_ddp_comp_id cur,
			      enum mtk_ddp_comp_id next);
#else
static inline void mtk_mmsys_ddp_connect(struct device *dev,
					 enum mtk_ddp_comp_id cur,
					 enum mtk_ddp_comp_id next)
{
}

static inline void mtk_mmsys_ddp_disconnect(struct device *dev,
					    enum mtk_ddp_comp_id cur,
					    enum mtk_ddp_comp_id next)
{
}
#endif

#endif /* __MTK_MMSYS_H */

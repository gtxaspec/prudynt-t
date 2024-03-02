/*
 * IMP System header file.
 *
 * Copyright (C) 2014 Ingenic Semiconductor Co.,Ltd
 */

#ifndef __IMP_SYSTEM_H__
#define __IMP_SYSTEM_H__

#include "imp_common.h"

#ifdef __cplusplus
#if __cplusplus
extern "C"
{
#endif
#endif /* __cplusplus */

/**
 * @file
 * IMP System header file
 */

/**
 * @defgroup imp IMP(Ingenic Media Platform)
 */

/**
 * @defgroup IMP_System
 * @ingroup imp
 * @brief System module including base function such as Bind etc.
 * @section concept 1 Concept
 * System control is mainly responsible for the connection of each module, and the definition of data flow's function. There we have some important concepts:

 *
 * @subsection device 1.1 Device
 * Device is to complete a set of (class) functions. Such as FrameSource generates Video source data output, Encoder realizes the function of video encoding or image encoding. FrameSouce and Encoder both are devices. \n
 * A Device is just a collection of concepts, and not a specific data flow node.
 *
 * @subsection group 1.2 Group
 * Group is the smallest unit of data input. A Device can have several Groups. Each Group has only one input, but multiple outputs (@ref output).
 *Group is also a specific function of the container, you can check about further explanation (@ref channel)
 *
 * @subsection output 1.3 Output
 * Output is Group's smallest unit of data output. A Group can have several outputs, and each Output can only generate one way of data.
 *
 * @subsection cell 1.4 Cell
 * Cell is a combination of Device, Group and Output infomation. See @ref IMPCell for further informations.\n
 * Cell is mainly to bind(@bind ). According to the definition of Device, Group, and Output, Output is the output node and Group is the input node.\n
 * When Binding, the output node refers to the output Cell's Output, and the input node refers to the input Cell's Group (so Cell as a data entry, Output is a meaningless value).
 *
 *subsection channel 1.5 Channel
 *
 * Channel usually refers to a single function of the unit. Specific function needs to be appointed while creating a Channel.\n
 * For example, The Channnel of the Encoder is used for H264 encoding or JPEG encoding. The specific algorithm type parameters are appointed when the channel is created.
 * For OSD, there is a similar concept with Channel, Region. A Region is a specific overlay area, it can be PIC (image), COVER (block), etc.\n
 *
 *For FrameSource, one Channel outputs one single original image, Channel FrameSource is actually Group
*
* Channel as a functional unit, usually requires Register to Group (except FrameSource), in order to receive data. The Channel once registered to Group, it will get the Group input.
 * For different Group Device, the Register Channel number is also different.
 *
 * @section bind 2 Bind
 * After binding two Groups, the source Group data will be automaticly sent to the destination Group.
 * Because the group is the smallest unit of data input and Output is the smallest unit of data output, therefore the srcCell's two parameters(deviceID, groupid) of IMP system bind (IMPCell *srcCell,     IMPCell *dstCell) . OutputID is effective
 * while dstCell only deviceID and groupID are valid, outputID is meaningless as a data entry. \n
 * the following figure is a simple example of Bind.
 *
 * @image HTML system_bind0.jpg
 * The image above, is the binding of one FrameSource's output to an Encoder's Group.
 * in the Encoder's Group registers two Channels, so Encoder Group is a two-way output: H264 and JPEG.
 *
 * Sample code in this situation:
 * @code
 * IMPCell fs_chn0 = {DEV_ID_FS, 0, 0};			//FrameSource deviceID:DEV_ID_FS groupID:0 outputID:0
 * IMPCell enc_grp0 = {DEV_ID_ENC, 0, 0};		//ENC deviceID:DEV_ID_ENC groupID:0 outputID:0, enc_grp0 here is meaningless.
 * int ret = IMP_System_Bind(&fs_chn0, &enc_grp0);
 * if (ret < 0)
 *     printf("Bind FrameSource Channel0 and Encoder Group0 failed\n");
 * @endcode
 *
 * Data streams are connected via Bind. So strategy may be different in different data-flow case. The following is a figure of typical case of dual-stream:
 *
 * @image html typical_application.png
 *
 * In the figure above, FrameSource has two outputs, Channel0 primary stream (1280x720) and Channel1 secondary stream (640x360).
 *
 *	*main stream:FrameSource's Channel0 Bind OSD Group.0，OSD Group.0 Bind Encoder Group.0。
 *		*OSD Group.0 registers two regions to display timestamp and string information respectively.
 *		*Encoder Group.0
 *
 *Two channels are registered for H264 encoding and JPEG encoding respectively. If the image size of the JPEG encoding channel is not equal to the input setting (Channel0 of FrameSource), it will be scaled (Software at T10) to capture the image at any resolution.
 *
 *	*second stream:FrameSource's Channel1 Bind IVS Group.0，IVS Group.0 Bind OSD Group.1，OSD Group.1 Bind Encoder Group.1.
 *		*IVS Group.0 registers a Channel for movement detection.
 *		*OSD Group.1 registers two regions to display timestamp and string information respectively.
 *		*Encoder Group.1 registers a Channel and encodes H264.
 *		*It is important to note that IVS Bind is placed before OSD because the OSD timestamp may cause incorrect IVS movement detection.
 *
 *
 *Reference code:
 * @image html system_bind2.png
 *
 * Sample code in this situation:\n
 * Bind the main stream:
 * @code
 * IMPCell fs_chn0 = {DEV_ID_FS, 0, 0};
 * IMPCell osd_grp0 = {DEV_ID_OSD, 0, 0};
 * IMPCell enc_grp0 = {DEV_ID_ENC, 0, 0};
 * int ret = IMP_System_Bind(&fs_chn0, &osd_grp0);
 * if (ret < 0)
 *     printf("Bind FrameSource Channel0 and OSD Group0 failed\n");
 *
 * int ret = IMP_System_Bind(&osd_grp0, &enc_grp0);
 * if (ret < 0)
 *     printf("Bind OSD Group0 and Encoder Group0 failed\n");
 * @endcode
 * Bind the second stream:
 * @code
 * IMPCell fs_chn1_output0 = {DEV_ID_FS, 1, 0};
 * IMPCell osd_grp1 = {DEV_ID_OSD, 1, 0};
 * IMPCell enc_grp1 = {DEV_ID_ENC, 1, 0};
 * IMPCell fs_chn1_output1 = {DEV_ID_FS, 1, 1};
 * IMPCell ivs_grp0 = {DEV_ID_IVS, 0, 0};
 *
 * int ret = IMP_System_Bind(&fs_chn1_output0, &osd_grp1);
 * if (ret < 0)
 *     printf("Bind FrameSource Channel1 and OSD Group1 failed\n");
 *
 * int ret = IMP_System_Bind(&osd_grp1, &enc_grp1);
 * if (ret < 0)
 *     printf("Bind OSD Group1 and Encoder Group1 failed\n");
 *
 * int ret = IMP_System_Bind(&fs_chn1_output1, &ivs_grp0);
 * if (ret < 0)
 *     printf("Bind FrameSource Channel1 Output1 and IVS failed\n");
 * @endcode
 *
 * @attention It is recommended that all Bind operations are performed during system initialization.
 * @attention After making the FrameSource enable, Bind and UnBind operation can not be dynamically called, you first need to disable FrameSource and then UnBind.
 * @attention DestroyGroup can be processed after "UnBind".
 *
 * Bind can be a tree structure, the following figure is an example
 *
 * @image html different_output.png
 * in the image above, the channel1(Group.1) of FrameSource's back-end binds two Groups, respectively from the Output.0 and Output.1 output data. In this case, the benefits of such binding is that IVS Group can work with OSD Group in parallel.
* @attention this kind of binding might have an impact on the ordinary mobile detection, so ordinary motion detection is not recommended by this way
 * @{
 */

/**
 * IMP Version definition.
 */
typedef struct {
	char aVersion[64];	/**< IMP Version */
} IMPVersion;

/**
 * @fn int IMP_System_Init(void)
 *
 * Initialize the IMP.
 *
 * @param None.
 *
 * @retval 0 Success.
 * @retval OtherValues Failure.
 *
 * @remarks After calling this API, the basis of data structure will be initialized, but does not initialize the hardware unit.
 *
 * @attention This interface must be initialized before any operation in IMP is required.
 */
int IMP_System_Init(void);

/**
 * @fn int IMP_System_Exit(void)
 *
 * Deinitialize the IMP.
 *
 * @param None.
 *
 * @retval 0 Success.
 * @retval OtherValues Failure.
 *
 * @remarks Memory and handlers will be freed after calling this API, also hardware will be closed.
 *
 * @attention After calling this API, if you want to use the IMP again, you need to reinitialize the IMP system
 */
int IMP_System_Exit(void);

/**
 * @fn int64_t IMP_System_GetTimeStamp(void)
 *
 * Get the current timestamp(unit is usec).
 *
 * @param None.
 *
 * @retval timestamp(usec).
 *
 * @remarks After the system initialization, the time stamp is initialized automatically.
 *
 * @attention None.
 */
int64_t IMP_System_GetTimeStamp(void);

/**
 * @fn int IMP_System_RebaseTimeStamp(int64_t basets)
 *
 * Set the timestamp(usec).
 *
 * @param[in] basets Base timestamp.
 *
 * @retval 0 Success.
 * @retval OtherValues Failure.
 *
 * @remarks None.
 *
 * @attention None.
 */
int IMP_System_RebaseTimeStamp(int64_t basets);

/**
 * @fn uint32_t IMP_System_ReadReg32(uint32_t u32Addr)
 *
 * Read the register(32bit) value.
 *
 * @param[in] regAddr Physical address register
 *
 * @retval Value of the the register(32bit).
 *
 * @remarks None.
 *
 * @attention None.
 */
uint32_t IMP_System_ReadReg32(uint32_t regAddr);

/**
 * @fn void IMP_System_WriteReg32(uint32_t regAddr, uint32_t value)
 *
 * Write the value to register(32bit).
 *
 * @param[in] regAddr Physical address
 * @param[in] value    Value to be written
 *
 * @retval None.
 *
 * @remarks None.
 *
 * @attention If you are not clear on the meaning of the register, please call this API carefully, or you may cause a system error.
 */
void IMP_System_WriteReg32(uint32_t regAddr, uint32_t value);

/**
 * @fn int IMP_System_GetVersion(IMPVersion *pstVersion)
 *
 * Get the version of IMP library.
 *
 * @param[out] pstVersion Pointer to IMPVersion.
 *
 * @retval 0 Success.
 * @retval OtherValues Failure.
 *
 * @remarks None.
 *
 * @attention None.
 */
int IMP_System_GetVersion(IMPVersion *pstVersion);

/**
 * @fn const char* IMP_System_GetCPUInfo(void)
 *
 * Get the infomation of CPU.
 *
 * @param None.
 *
 * @retval String of IMPVersion.
 *
 * @remarks The return value is a string of the CPU model type  e.g. "T10", "T10-Lite" etc.
 *
 * @attention None.
 */
const char* IMP_System_GetCPUInfo(void);

/**
 * @fn int IMP_System_Bind(IMPCell *srcCell, IMPCell *dstCell)
 *
 * Bind the source Cell and the destination Cell.
 *
 * @param[in] srcCell The source Cell pointer.
 * @param[in] dstCell The destination Cell pointer.
 *
 * @retval 0 Success.
 * @retval OtherValues Failure.
 *
 * @remarks According to the concepts of Device, Group and Output, Every Device might have several Groups, every Group several Outputs,
 * Group is the Device input interface, Output is the output interface, so Binding will be the process of linking the Device's Outputs to the Device's inputs.
 * @remarks After Bindding successfully the source Cell will deliver the data to the destination Cell automatically.
 *
 * @attention None.
 */
int IMP_System_Bind(IMPCell *srcCell, IMPCell *dstCell);

/**
 * @fn int IMP_System_UnBind(IMPCell *srcCell, IMPCell *dstCell)
 *
 * UnBind source Cell and the destination Cell.
 *
 * @param[in] srcCell The source Cell pointer.
 * @param[in] dstCell The destination Cell pointer.
 *
 *
 * @retval 0 Success.
 * @retval OtherValues Failure.
 *
 * @remarks None.
 *
 * @attention None.
 */
int IMP_System_UnBind(IMPCell *srcCell, IMPCell *dstCell);

/**
 * @fn int IMP_System_GetBindbyDest(IMPCell *dstCell, IMPCell *srcCell)
 *
 * Gets the source Cell information that is bound to the destination Cell.
 *
 * @param[out] srcCell The source Cell pointer.
 * @param[in] dstCell The destination Cell pointer.
 *
 *
 * @retval 0 Success.
 * @retval OtherValues Failure.
 *
 * @remarks None.
 *
 * @attention None.
 */
int IMP_System_GetBindbyDest(IMPCell *dstCell, IMPCell *srcCell);

/**
 * @brief IMP_System_MemPoolRequest(int poolId, size_t size, const char *name);
 *
 * Request memory pool on Rmem
 *
 * @param[in] poolId     ID of the application.
 * @param[in] size		 Request mempool size.
 * @param[in] name		 The Pool name.
 *
 * @retval	0			success.
 * @retval  OtherValues Failure.
 *
 * @remarks
 * Video memory pool mainly provides large physical continuous memory management function
 * for media business, which is responsible for memory allocation and recovery, and gives
 * full play to the role of memory buffer pool, so that physical memory resources can be
 * used reasonably in various media processing modules. The video memory pool is based on
 * the original reserved memory rmem (the page table is not created by the kernel) for
 * large memory management. Each application for a memory pool is physical continuous
 * memory, and then applying for memory on this memory pool is also applying for physical
 * continuous memory. If a memory pool is used, the size of the memory pool must be
 * configured before system initialization. The size and number of memory pool
 * applications vary according to different services.
 *
 * @attention	none.
 */
int IMP_System_MemPoolRequest(int poolId, size_t size, const char *name);

/**
 * @brief IMP_System_MemPoolFree
 *
 * @param[in] poolId:  release mempool rmem area memory
 *
 * @retval:  0, success
 * @retval:  OhterValues,  failure
 *
 * @attention: Memory pool release: it is forbidden to release when the code stream is running. It can be released only after all resources exit
 * In addition, in the SDK, the related memory applications do not go to the memory pool. If frequent applications are released,
 * the applications may be caused by memory fragmentation
 * In case of failure, it is generally recommended to release the memory pool only when rmem memory is required for other purposes
 */
int IMP_System_MemPoolFree(int poolId);

/**
 * @}
 */

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* __IMP_SYSTEM_H__ */

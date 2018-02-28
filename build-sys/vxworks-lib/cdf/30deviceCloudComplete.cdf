/* 30deviceCloudComplete.cdf - Device Cloud component descriptor file */
/*
 * Copyright (C) 2018 Wind River Systems, Inc. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at:
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 */

#if 0
Component INCLUDE_DEVICE_CLOUD_COMPLETE_KERNEL {
        NAME            Device Cloud complete kernel task
        SYNOPSIS        This component invokes the Device Cloud complete application as a kernel task
        _CHILDREN       SELECT_DEVICE_CLOUD_AGENT_APP
        ARCHIVE         libdeviceCloudComplete.a
        CONFIGLETTES    deviceCloudCompleteCfg.c
        PROTOTYPE       void deviceCloudCompleteStart(void);
        INIT_RTN        deviceCloudCompleteStart();
        _INIT_ORDER     usrRoot
        INIT_AFTER      usrToolsInit
        INIT_BEFORE     INCLUDE_USER_APPL
        REQUIRES        INCLUDE_DEVICE_CLOUD_AGENT
}

Component INCLUDE_DEVICE_CLOUD_COMPLETE_RTP {
        NAME            Device Cloud complete RTP
        SYNOPSIS        This component invokes the Device Cloud complete application as an RTP
        _CHILDREN       SELECT_DEVICE_CLOUD_AGENT_APP
        CONFIGLETTES    deviceCloudCompleteRtpCfg.c
        PROTOTYPE       void deviceCloudCompleteRtp(void);
        INIT_RTN        deviceCloudCompleteRtp();
        _INIT_ORDER     usrRoot
        INIT_AFTER      usrToolsInit
        INIT_BEFORE     INCLUDE_USER_APPL
        REQUIRES        INCLUDE_DEVICE_CLOUD_AGENT
}
#endif

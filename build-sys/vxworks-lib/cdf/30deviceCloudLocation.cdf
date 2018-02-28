/* 30deviceCloudLocation.cdf - Device Cloud component descriptor file */
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

Component INCLUDE_DEVICE_CLOUD_LOCATION_KERNEL {
        NAME            Device Cloud location kernel task
        SYNOPSIS        This component invokes the Device Cloud location application as a kernel task
        _CHILDREN       SELECT_DEVICE_CLOUD_AGENT_APP
        ARCHIVE         libdeviceCloudLocation.a
        CONFIGLETTES    deviceCloudLocationCfg.c
        PROTOTYPE       void deviceCloudLocationStart(void);
        INIT_RTN        deviceCloudLocationStart();
        _INIT_ORDER     usrRoot
        INIT_AFTER      usrToolsInit
        INIT_BEFORE     INCLUDE_USER_APPL
        REQUIRES        INCLUDE_DEVICE_CLOUD_AGENT
}

#if 0
Component INCLUDE_DEVICE_CLOUD_LOCATION_RTP {
        NAME            Device Cloud location RTP
        SYNOPSIS        This component invokes the Device Cloud location application as an RTP
        _CHILDREN       SELECT_DEVICE_CLOUD_AGENT_APP
        CONFIGLETTES    deviceCloudLocationRtpCfg.c
        PROTOTYPE       void deviceCloudLocationRtp(void);
        INIT_RTN        deviceCloudLocationRtp();
        _INIT_ORDER     usrRoot
        INIT_AFTER      usrToolsInit
        INIT_BEFORE     INCLUDE_USER_APPL
        REQUIRES        INCLUDE_DEVICE_CLOUD_AGENT
}
#endif

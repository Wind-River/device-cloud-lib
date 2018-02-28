/* 30deviceCloudManager.cdf - Device Cloud component descriptor file */
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

Component INCLUDE_DEVICE_CLOUD_MANAGER_KERNEL {
        NAME            Device Cloud manager kernel task
        SYNOPSIS        This component invokes the Device Cloud manager application as a kernel task
        _CHILDREN       SELECT_DEVICE_CLOUD_AGENT_APP
        _DEFAULTS       SELECT_DEVICE_CLOUD_AGENT_APP
        ARCHIVE         libdeviceCloudManager.a
        CONFIGLETTES    deviceCloudManagerCfg.c
        PROTOTYPE       void deviceCloudManagerStart(void);
        INIT_RTN        deviceCloudManagerStart();
        _INIT_ORDER     usrRoot
        INIT_AFTER      usrAppInit
        REQUIRES        INCLUDE_DEVICE_CLOUD_AGENT
}

#if 0
Component INCLUDE_DEVICE_CLOUD_MANAGER_RTP {
        NAME            Device Cloud manager RTP
        SYNOPSIS        This component invokes the Device Cloud manager application as an RTP
        _CHILDREN       SELECT_DEVICE_CLOUD_AGENT_APP
        CONFIGLETTES    deviceCloudManagerRtpCfg.c
        PROTOTYPE       void deviceCloudManagerRtp(void);
        INIT_RTN        deviceCloudManagerRtp();
        _INIT_ORDER     usrRoot
        INIT_AFTER      usrRtpAppInit
        REQUIRES        INCLUDE_DEVICE_CLOUD_AGENT
}
#endif

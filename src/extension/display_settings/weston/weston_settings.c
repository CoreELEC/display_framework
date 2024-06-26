/*
 * Copyright (c) 2023 Amlogic, Inc. All rights reserved.
 * Description:
 */

#include <assert.h>
#include <inttypes.h>
#include <stdbool.h>
#include <sys/ioctl.h>
#include <drm_fourcc.h>
#include <sys/mman.h>
#include <pthread.h>
#include "xf86drm.h"
#include "xf86drmMode.h"
#include <signal.h>
#include <drm.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <linux/string.h>
#include "../display_settings.h"
#include "libdrm_meson/meson_drm_settings.h"
#define DEFAULT_CARD "/dev/dri/card0"
#include "libdrm_meson/meson_drm_log.h"

#define CMDBUF_SIZE 256
static int wstDisplaySendMessage(char* property,char *response);
static int getValueFromOutputString(const char *str, const char *key);

static int getValueFromOutputString(const char *str, const char *key) {
    int value = -1;
    if (str != NULL) {
        DEBUG("%s %d get output string %s",__FUNCTION__,__LINE__,str);
        char *temp = strstr(str, key);
        if (temp != NULL) {
            temp += strlen(key);
            value = atoi(temp);
        }
    }
    return value;
}

static int wstDisplaySendMessage(char* property,char *response) {
    int ret = -1;
    DEBUG("%s %d send message parameters %s ", __FUNCTION__, __LINE__, property);
    if (property) {
        do {
            char cmdBuf[CMDBUF_SIZE] = {'\0'};
            snprintf(cmdBuf, sizeof(cmdBuf) - 1, "drm-helper-client %s", property);
            DEBUG("%s %d Executing '%s' \n", __FUNCTION__, __LINE__, cmdBuf);

            FILE* fp = popen(cmdBuf, "r");
            if (NULL != fp) {
                char output[CMDBUF_SIZE] = {'\0'};
                while (fgets(output, sizeof(output) - 1, fp) != NULL) {
                    DEBUG("%s %d output: %s\n", __FUNCTION__, __LINE__, output);
                }
                ret = 0;
                strcpy(response, output);
                pclose(fp);
            } else {
                ERROR("%s %d Command execution failed", __FUNCTION__, __LINE__);
                ret = -1;
            }
        } while (ret != 0);
    } else {
        ERROR("%s %d Invalid property", __FUNCTION__, __LINE__);
        ret = -1;
    }
    return ret;
}

int setDisplayHDCPEnable(int enable, DISPLAY_CONNECTOR_TYPE connType) {
    int ret = -1;
    int rc = -1;
    int connId = -1;
    char resp[CMDBUF_SIZE] = {'\0'};
    char cmdBuf[CMDBUF_SIZE] = {'\0'};
    char* prop_name = NULL;
    connId = meson_drm_GetConnectorId(connType);
    DEBUG(" %s %d weston set hdcp enable %d connId %d connType %d",__FUNCTION__,__LINE__,
                                       enable,connId,connType);
    if (connId > 0) {
        prop_name = meson_drm_GetPropName(ENUM_MESON_DRM_PROP_CONTENT_PROTECTION);
        if (prop_name == NULL) {
            ERROR("%s %d meson_drm_GetPropName return NULL",__FUNCTION__,__LINE__);
            goto out;
        }
        DEBUG("%s %d get prop name %s",__FUNCTION__,__LINE__, prop_name);
        snprintf(cmdBuf, sizeof(cmdBuf)-1, "-s \"%s\"=%d", prop_name, enable);
        rc = wstDisplaySendMessage(cmdBuf,resp);
        if (rc >= 0) {
            ret = 0;
       } else {
            ERROR("%s %d send message fail",__FUNCTION__,__LINE__);
       }
    } else {
        ERROR("%s %d meson_drm_GetConnectorId return fail",__FUNCTION__,__LINE__);
    }
out:
    if (prop_name) {
        free(prop_name);
    }
    return ret;
}

int setDisplayAVMute(int mute, DISPLAY_CONNECTOR_TYPE connType) {
    int ret = -1;
    int connId = -1;
    int rc = -1;
    char resp[CMDBUF_SIZE] = {'\0'};
    char cmdBuf[CMDBUF_SIZE] = {'\0'};
    char* prop_name = NULL;
    connId = meson_drm_GetConnectorId(connType);
    DEBUG("%s %d weston set mute value %d connId %d connType %d",__FUNCTION__,__LINE__,
                                      mute,connId,connType);
    if (connId > 0) {
        prop_name = meson_drm_GetPropName(ENUM_MESON_DRM_PROP_HDMI_ENABLE);
        if (prop_name == NULL) {
            ERROR("%s %d meson_drm_GetPropName return NULL",__FUNCTION__,__LINE__);
            goto out;
        }
        DEBUG("%s %d get prop name %s",__FUNCTION__,__LINE__, prop_name);
        snprintf(cmdBuf, sizeof(cmdBuf)-1, "-s \"%s\"=%d", prop_name, mute);
        rc = wstDisplaySendMessage(cmdBuf,resp);
        if (rc >= 0) {
            ret = 0;
        } else {
            ERROR("%s %d send message fail",__FUNCTION__,__LINE__);
        }
    } else {
        ERROR("%s %d meson_drm_GetConnectorId return fail",__FUNCTION__,__LINE__);
    }
out:
    if (prop_name) {
        free(prop_name);
    }
    return ret;
}

int setDisplayHDCPContentType(ENUM_DISPLAY_HDCP_Content_Type HDCPType, DISPLAY_CONNECTOR_TYPE connType) {
    int ret = -1;
    int rc = -1;
    int connId = -1;
    char resp[CMDBUF_SIZE] = {'\0'};
    char cmdBuf[CMDBUF_SIZE] = {'\0'};
    char* prop_name = NULL;
    connId = meson_drm_GetConnectorId(connType);
    DEBUG(" %s %d weston set hdcp content type %d connId %d connType %d",__FUNCTION__,__LINE__,
                              HDCPType,connId,connType);
    if (connId > 0) {
        prop_name = meson_drm_GetPropName(ENUM_MESON_DRM_PROP_HDCP_VERSION);
        if (prop_name == NULL) {
            ERROR("%s %d meson_drm_GetPropName return NULL",__FUNCTION__,__LINE__);
            goto out;
        }
        DEBUG("%s %d get prop name %s",__FUNCTION__,__LINE__, prop_name);
        snprintf(cmdBuf, sizeof(cmdBuf)-1, "-s \"%s\"=%d", prop_name, HDCPType);
        rc = wstDisplaySendMessage(cmdBuf,resp);
        if (rc >= 0) {
            ret = 0;
       } else {
            ERROR("%s %d send message fail",__FUNCTION__,__LINE__);
       }
    } else {
        ERROR("%s %d meson_drm_GetConnectorId return fail",__FUNCTION__,__LINE__);
    }
out:
    if (prop_name) {
        free(prop_name);
    }
    return ret;
}

int setDisplayDvEnable(int dvEnable, DISPLAY_CONNECTOR_TYPE connType) {
    int ret = -1;
    int rc = -1;
    char resp[CMDBUF_SIZE] = {'\0'};
    char cmdBuf[CMDBUF_SIZE] = {'\0'};
    uint32_t crtcId = -1;
    char* prop_name = NULL;
    crtcId = meson_drm_GetCrtcId(connType);
    DEBUG(" %s %d weston set DvEnable %d crtcId %d connType %d",__FUNCTION__,__LINE__,
                                dvEnable,crtcId,connType);
    if (crtcId > 0) {
        prop_name = meson_drm_GetPropName(ENUM_MESON_DRM_PROP_DOLBY_VISION_ENABLE);
        if (prop_name == NULL) {
            ERROR("%s %d meson_drm_GetPropName return NULL",__FUNCTION__,__LINE__);
            goto out;
        }
        DEBUG("%s %d get prop name %s",__FUNCTION__,__LINE__, prop_name);
        snprintf(cmdBuf, sizeof(cmdBuf)-1, "-s \"%s\"=%d", prop_name, dvEnable);
        rc = wstDisplaySendMessage(cmdBuf,resp);
        if (rc >= 0) {
            ret = 0;
       } else {
           ERROR("%s %d send message fail",__FUNCTION__,__LINE__);
       }
    } else {
        ERROR("%s %d meson_drm_GetCrtcId return fail",__FUNCTION__,__LINE__);
    }
out:
    if (prop_name) {
        free(prop_name);
    }
    return ret;
}

int setDisplayActive(int active, DISPLAY_CONNECTOR_TYPE connType) {
    int ret = -1;
    int rc = -1;
    char resp[CMDBUF_SIZE] = {'\0'};
    char cmdBuf[CMDBUF_SIZE] = {'\0'};
    uint32_t crtcId = -1;
    char* prop_name = NULL;
    crtcId = meson_drm_GetCrtcId(connType);
    DEBUG(" %s %d weston set active %d crtcId %d connType %d",__FUNCTION__,__LINE__,
                              active,crtcId, connType);
    if (crtcId > 0) {
        prop_name = meson_drm_GetPropName(ENUM_MESON_DRM_PROP_ACTIVE);
        if (prop_name == NULL) {
            ERROR("%s %d meson_drm_GetPropName return NULL",__FUNCTION__,__LINE__);
            goto out;
        }
        DEBUG("%s %d get prop name %s",__FUNCTION__,__LINE__, prop_name);
        snprintf(cmdBuf, sizeof(cmdBuf)-1, "-s \"%s\"=%d", prop_name, active);
        rc = wstDisplaySendMessage(cmdBuf,resp);
        if (rc >= 0) {
            ret = 0;
       } else {
            ERROR("%s %d send message fail",__FUNCTION__,__LINE__);
       }
    } else {
        ERROR("%s %d meson_drm_GetCrtcId return fail",__FUNCTION__,__LINE__);
    }
out:
    if (prop_name) {
        free(prop_name);
    }
    return ret;
}

int setDisplayVrrEnabled(int VrrEnable, DISPLAY_CONNECTOR_TYPE connType) {
    int ret = -1;
    int rc = -1;
    char resp[CMDBUF_SIZE] = {'\0'};
    char cmdBuf[CMDBUF_SIZE] = {'\0'};
    uint32_t crtcId = -1;
    char* prop_name = NULL;
    crtcId = meson_drm_GetCrtcId(connType);
    DEBUG(" %s %d weston set VrrEnable %d crtcId %d connType %d",__FUNCTION__,__LINE__,
                            VrrEnable,crtcId,connType);
    if ( crtcId > 0 ) {
        prop_name = meson_drm_GetPropName(ENUM_MESON_DRM_PROP_VRR_ENABLED);
        if (prop_name == NULL) {
            ERROR("%s %d meson_drm_GetPropName return NULL",__FUNCTION__,__LINE__);
            goto out;
        }
        DEBUG("%s %d get prop name %s",__FUNCTION__,__LINE__, prop_name);
        snprintf(cmdBuf, sizeof(cmdBuf)-1, "-s \"%s\"=%d", prop_name, VrrEnable);
        rc = wstDisplaySendMessage(cmdBuf,resp);
        if ( rc >= 0 ) {
            ret = 0;
        } else {
            ERROR("%s %d send message fail",__FUNCTION__,__LINE__);
        }
    } else {
        ERROR("%s %d meson_drm_GetCrtcId return fail",__FUNCTION__,__LINE__);
    }
out:
    if (prop_name) {
        free(prop_name);
    }
    return ret;
}

int setDisplayMode(DisplayModeInfo* modeInfo,DISPLAY_CONNECTOR_TYPE connType) {
    int ret = -1;
    char modeSet[CMDBUF_SIZE] = {'\0'};
    char resp[CMDBUF_SIZE] = {'\0'};
    DisplayModeInfo* modes = NULL;
    int count = 0;
    bool found = false;
    int rc = -1;
    int fd = meson_open_drm();
    if (modeInfo == NULL) {
        ERROR("%s %d invalid parameter return",__FUNCTION__,__LINE__);
        goto out;
    }
    DEBUG("%s %d weston set modeInfo %dx%d%c%dhz",__FUNCTION__,__LINE__, modeInfo->w, modeInfo->h, (modeInfo->interlace == 0? 'p':'i') , modeInfo->vrefresh);
    snprintf(modeSet, sizeof(modeSet)-1, "-c %dx%d%c@%d", modeInfo->w, modeInfo->h,(modeInfo->interlace == 0? 'p':'i'), modeInfo->vrefresh);
    rc = wstDisplaySendMessage(modeSet,resp);
    if ( rc >= 0 ) {
        ret = 0;
    } else {
        ERROR("%s %d send message fail",__FUNCTION__,__LINE__);
    }

out:
    if (modes) {
        free(modes);
    }
    meson_close_drm(fd);
    return ret;
}

int setDisplayDummyMode(DISPLAY_CONNECTOR_TYPE connType) {
    int ret = -1;
    char resp[CMDBUF_SIZE] = {'\0'};
    char modeSet[CMDBUF_SIZE] = {'\0'};
    int rc = -1;
    snprintf(modeSet, sizeof(modeSet)-1, "-c %s", "dummy_l");
    DEBUG("%s %d weston set mode %s",__FUNCTION__,__LINE__, modeSet);
    rc = wstDisplaySendMessage(modeSet,resp);
    if ( rc >= 0 ) {
        ret = 0;
    } else {
        ERROR("%s %d send message fail",__FUNCTION__,__LINE__);
    }
    return ret;
}

int setDisplayDvMode(int dvmode,DISPLAY_CONNECTOR_TYPE connType) {
    int ret = -1;
    int crtcId = -1;
    int rc = -1;
    char resp[CMDBUF_SIZE] = {'\0'};
    char cmdBuf[CMDBUF_SIZE] = {'\0'};
    char* prop_name = NULL;
    crtcId = meson_drm_GetCrtcId(connType);
    DEBUG("%s %d weston set dv mode %d crtcId %d connType %d",__FUNCTION__,__LINE__,
                               dvmode,crtcId,connType);
    if (crtcId > 0) {
        prop_name = meson_drm_GetPropName(ENUM_MESON_DRM_PROP_DV_MODE);
        if (prop_name == NULL) {
            ERROR("%s %d meson_drm_GetPropName return NULL",__FUNCTION__,__LINE__);
            goto out;
        }
        DEBUG("%s %d get prop name %s",__FUNCTION__,__LINE__, prop_name);
        snprintf(cmdBuf, sizeof(cmdBuf)-1, "-s \"%s\"=%d", prop_name, dvmode);
        rc = wstDisplaySendMessage(cmdBuf,resp);
        if (rc >= 0) {
            ret = 0;
        } else {
            ERROR("%s %d send message fail",__FUNCTION__,__LINE__);
        }
    } else {
        ERROR("%s %d meson_drm_GetCrtcId return fail",__FUNCTION__,__LINE__);
    }
out:
    if (prop_name) {
        free(prop_name);
    }
    return ret;
}

int setDisplayCvbsAVMute(bool mute) {
    int ret = -1;
    int connId = -1;
    int rc = -1;
    char resp[CMDBUF_SIZE] = {'\0'};
    char cmdBuf[CMDBUF_SIZE] = {'\0'};
    char* prop_name = NULL;
    connId = meson_drm_GetConnectorId(MESON_CONNECTOR_CVBS);
    DEBUG("%s %d weston set cvbs mute value %d connId %d",__FUNCTION__,__LINE__,
                                      mute,connId);
    if (connId > 0) {
        prop_name = meson_drm_GetPropName(ENUM_MESON_DRM_CVBS_PROP_AVMUTE);
        if (prop_name == NULL) {
            ERROR("%s %d meson_drm_GetPropName return NULL",__FUNCTION__,__LINE__);
            goto out;
        }
        DEBUG("%s %d get prop name %s",__FUNCTION__,__LINE__, prop_name);
        snprintf(cmdBuf, sizeof(cmdBuf)-1, "-s \"%s\"=%d", prop_name, mute);
        rc = wstDisplaySendMessage(cmdBuf,resp);
        if (rc >= 0) {
            ret = 0;
        } else {
            ERROR("%s %d send message fail",__FUNCTION__,__LINE__);
        }
    } else {
        ERROR("%s %d meson_drm_GetConnectorId return fail",__FUNCTION__,__LINE__);
    }
out:
    if (prop_name) {
        free(prop_name);
    }
}

int setDisplayFracRatePolicy(int value, DISPLAY_CONNECTOR_TYPE connType) {
    int ret = -1;
    int connId = -1;
    int rc = -1;
    char resp[CMDBUF_SIZE] = {'\0'};
    char cmdBuf[CMDBUF_SIZE] = {'\0'};
    char* prop_name = NULL;
    connId = meson_drm_GetConnectorId(connType);
    DEBUG(" %s %d weston set frac rate policy %d connId %d connType %d",__FUNCTION__,__LINE__,
                                               value, connId, connType);
    if (connId > 0) {
        prop_name = meson_drm_GetPropName(ENUM_MESON_DRM_CONNECTOR_FRAC_RATE_POLICY);
        if (prop_name == NULL) {
            ERROR("%s %d meson_drm_GetPropName return NULL",__FUNCTION__,__LINE__);
            goto out;
        }
        DEBUG("%s %d get prop name %s",__FUNCTION__,__LINE__, prop_name);
        snprintf(cmdBuf, sizeof(cmdBuf)-1, "-s \"%s\"=%d", prop_name, value);
        rc = wstDisplaySendMessage(cmdBuf,resp);
        if (rc >= 0) {
            ret = 0;
        } else {
            ERROR("%s %d send message fail",__FUNCTION__,__LINE__);
        }
    } else {
        ERROR("%s %d meson_drm_GetConnectorId return fail",__FUNCTION__,__LINE__);
    }
out:
    if (prop_name) {
        free(prop_name);
    }
    return ret;
}

int setDisplayColorSpacedDepth(uint32_t colorDepth, ENUM_DISPLAY_COLOR_SPACE colorSpace,
                                    DISPLAY_CONNECTOR_TYPE connType) {
    int ret = -1;
    int connId = -1;
    char cmdBuf[CMDBUF_SIZE] = {'\0'};
    char* space_prop_name = NULL;
    char* depth_prop_name = NULL;
    char resp[CMDBUF_SIZE] = {'\0'};
    struct mesonConnector* conn = NULL;
    char* str = NULL;
    switch (colorSpace)
    {
        case 0:
            str = "DISPLAY_COLOR_SPACE_RGB";
            break;
        case 1:
            str = "DISPLAY_COLOR_SPACE_YCBCR422";
            break;
        case 2:
            str = "DISPLAY_COLOR_SPACE_YCBCR444";
            break;
        case 3:
            str = "DISPLAY_COLOR_SPACE_YCBCR420";
            break;
        default:
            str = "DISPLAY_COLOR_SPACE_RESERVED";
            break;
    }
    DEBUG("%s %d weston set colorDepth: %d colorSpace: %s",__FUNCTION__,__LINE__,colorDepth,str);
    connId = meson_drm_GetConnectorId(connType);
    if (connId > 0) {
        space_prop_name = meson_drm_GetPropName(ENUM_MESON_DRM_PROP_COLOR_SPACE);
        depth_prop_name = meson_drm_GetPropName(ENUM_MESON_DRM_PROP_COLOR_DEPTH);
        if (space_prop_name == NULL || depth_prop_name == NULL) {
            ERROR("%s %d meson_drm_GetPropName return NULL",__FUNCTION__,__LINE__);
            goto out;
        }
        DEBUG("%s %d depth_prop_name: %s space_prop_name: %s ",__FUNCTION__,__LINE__,depth_prop_name,space_prop_name);
        snprintf(cmdBuf, sizeof(cmdBuf)-1, "-s \"%s\"=%d -s \"%s\"=%d", depth_prop_name, colorDepth,space_prop_name, colorSpace);
        wstDisplaySendMessage(cmdBuf,resp);
        ret = 0;
    } else {
        ERROR("%s %d meson_drm_GetConnectorId return fail",__FUNCTION__,__LINE__);
    }
out:
    if (space_prop_name) {
        free(space_prop_name);
    }
    if (depth_prop_name) {
        free(depth_prop_name);
    }
    return ret;
}

int setDisplayHDRPolicy(ENUM_DISPLAY_HDR_POLICY hdrPolicy, DISPLAY_CONNECTOR_TYPE connType) {
    int ret = -1;
    int crtcId = -1;
    int rc = -1;
    char* hdrpolicy_name = NULL;
    char* force_output_name = NULL;
    char resp[CMDBUF_SIZE] = {'\0'};
    char cmdBuf[CMDBUF_SIZE] = {'\0'};
    ENUM_DISPLAY_FORCE_MODE forcemode = DISPLAY_UNKNOWN_FMT;
    DEBUG("%s %d weston set hdr policy %d",__FUNCTION__,__LINE__,hdrPolicy);
    crtcId = meson_drm_GetCrtcId(connType);
    if (crtcId > 0) {
        hdrpolicy_name = meson_drm_GetPropName(ENUM_MESON_DRM_PROP_HDR_POLICY);
        force_output_name = meson_drm_GetPropName(ENUM_MESON_DRM_PROP_TX_HDR_OFF);
        if (hdrpolicy_name == NULL || force_output_name == NULL) {
            ERROR("%s %d meson_drm_GetPropName return NULL",__FUNCTION__,__LINE__);
            goto out;
        }
        DEBUG("%s %d hdrpolicy_name %s force_output_name %s",__FUNCTION__,__LINE__, hdrpolicy_name,force_output_name);
        if (hdrPolicy == DISPLAY_HDR_POLICY_FOLLOW_FORCE_MODE) {
            forcemode = DISPLAY_BT709;
            snprintf(cmdBuf, sizeof(cmdBuf)-1, "-s \"%s\"=%d -s \"%s\"=%d", hdrpolicy_name, hdrPolicy,force_output_name, forcemode);
            DEBUG("%s %d hdrPolicy property: %s:%d forcemode property: %s:%d",__FUNCTION__,__LINE__,
                   hdrpolicy_name, hdrPolicy,force_output_name, forcemode);
            rc = wstDisplaySendMessage(cmdBuf,resp);
            if (rc >= 0) {
                ret = 0;
            } else {
                ERROR("%s %d send message fail",__FUNCTION__,__LINE__);
            }
        } else {
            forcemode = DISPLAY_UNKNOWN_FMT;
            snprintf(cmdBuf, sizeof(cmdBuf)-1, "-s \"%s\"=%d -s \"%s\"=%d", hdrpolicy_name, hdrPolicy,force_output_name, forcemode);
            DEBUG("%s %d hdrPolicy property: %s:%d forcemode property: %s:%d",__FUNCTION__,__LINE__,
                   hdrpolicy_name, hdrPolicy, force_output_name, forcemode);
            rc = wstDisplaySendMessage(cmdBuf,resp);
            if (rc >= 0) {
                ret = 0;
            } else {
                ERROR("%s %d send message fail",__FUNCTION__,__LINE__);
            }
        }
    } else {
        ERROR("%s %d meson_drm_GetCrtcId return fail",__FUNCTION__,__LINE__);
    }
out:
    if (force_output_name) {
        free(force_output_name);
    }
    if (hdrpolicy_name) {
        free(hdrpolicy_name);
    }
    return ret;;
}

int setDisplayAutoMode(DISPLAY_CONNECTOR_TYPE connType) {
    int ret = -1;
    char resp[CMDBUF_SIZE] = {'\0'};
    char modeSet[CMDBUF_SIZE] = {'\0'};
    int rc = -1;
    snprintf(modeSet, sizeof(modeSet)-1, "-c %s", "automode");
    rc = wstDisplaySendMessage(modeSet,resp);
    if ( rc >= 0 ) {
        ret = 0;
    } else {
        ERROR("%s %d send message fail",__FUNCTION__,__LINE__);
    }

    return ret;
}

int setDisplayAspectRatioValue(ENUM_DISPLAY_ASPECT_RATIO ASPECTRATIO, DISPLAY_CONNECTOR_TYPE connType) {
    int ret = -1;
    int connId = -1;
    int rc = -1;
    char resp[CMDBUF_SIZE] = {'\0'};
    char cmdBuf[CMDBUF_SIZE] = {'\0'};
    char* prop_name = NULL;
    connId = meson_drm_GetConnectorId(connType);
    DEBUG(" %s %d weston set aspect ratio Value %d connId %d connType %d",__FUNCTION__,__LINE__,
                                               ASPECTRATIO, connId, connType);
    if (connId > 0) {
        prop_name = meson_drm_GetPropName(ENUM_MESON_DRM_PROP_ASPECT_RATIO);
        if (prop_name == NULL) {
            ERROR("%s %d meson_drm_GetPropName return NULL",__FUNCTION__,__LINE__);
            goto out;
        }
        DEBUG("%s %d get prop name %s",__FUNCTION__,__LINE__, prop_name);
        snprintf(cmdBuf, sizeof(cmdBuf)-1, "-s \"%s\"=%d", prop_name, ASPECTRATIO);
        rc = wstDisplaySendMessage(cmdBuf,resp);
        if (rc >= 0) {
            ret = 0;
        } else {
            ERROR("%s %d send message fail",__FUNCTION__,__LINE__);
        }
    } else {
        ERROR("%s %d meson_drm_GetConnectorId return fail",__FUNCTION__,__LINE__);
    }
out:
    if (prop_name) {
        free(prop_name);
    }
    return ret;
}

int setDisplayFracMode(DisplayModeInfo* modeInfo, int value, DISPLAY_CONNECTOR_TYPE connType) {
    int ret = -1;
    char modeSet[CMDBUF_SIZE] = {'\0'};
    int count = 0;
    int rc = -1;
    int connId = -1;
    char cmdBuf[CMDBUF_SIZE] = {'\0'};
    char resp[CMDBUF_SIZE] = {'\0'};
    char* prop_name = NULL;
    int fd = meson_open_drm();
    if (modeInfo == NULL) {
        ERROR("%s %d invalid parameter return",__FUNCTION__,__LINE__);
        goto out;
    }
    connId = meson_drm_GetConnectorId(connType);
    DEBUG("%s %d weston set modeInfo %dx%d%c%dhz frac rate policy %d",__FUNCTION__,__LINE__, modeInfo->w,
    modeInfo->h, (modeInfo->interlace == 0? 'p':'i') , modeInfo->vrefresh, value);
    if (connId > 0) {
        prop_name = meson_drm_GetPropName(ENUM_MESON_DRM_CONNECTOR_FRAC_RATE_POLICY);
        if (prop_name == NULL) {
            ERROR("%s %d meson_drm_GetPropName return NULL",__FUNCTION__,__LINE__);
            goto out;
        }
        DEBUG("%s %d get prop name %s",__FUNCTION__,__LINE__, prop_name);

        snprintf(modeSet, sizeof(modeSet)-1, "-r \"set mode %dx%d%c@%d properties %s=%d\"", modeInfo->w, modeInfo->h,
                   (modeInfo->interlace == 0? 'p':'i'), modeInfo->vrefresh, prop_name,value);
        rc = wstDisplaySendMessage(modeSet,resp);
        if (rc >= 0) {
            ret = 0;
        } else {
            ERROR("%s %d send message fail",__FUNCTION__,__LINE__);
        }
    } else {
        ERROR("%s %d meson_drm_GetConnectorId return fail",__FUNCTION__,__LINE__);
    }
out:
    if (prop_name) {
        free(prop_name);
    }
    meson_close_drm(fd);
    return ret;
}

int setDisplayModeAttr(DisplayModeInfo* modeInfo,uint32_t colorDepth,
                     ENUM_DISPLAY_COLOR_SPACE colorSpace,DISPLAY_CONNECTOR_TYPE connType) {
    return  0;
}

int getDisplayIsBestMode(int* value) {
    return 0;
}

int setDisplayEnabled(int enabled) {
    int ret = -1;
    int rc = -1;
    char resp[CMDBUF_SIZE] = {'\0'};
    char cmdBuf[CMDBUF_SIZE] = {'\0'};

    snprintf(cmdBuf, sizeof(cmdBuf) - 1, "-r \"set display enable %d\"", enabled);
    rc = wstDisplaySendMessage(cmdBuf,resp);
    if (rc >= 0) {
        ret = 0;
    } else {
        ERROR("%s %d send message fail", __FUNCTION__, __LINE__);
    }
    return ret;
}

int getDisplayEnabled(int* enabled) {
    return 0;
}

int getDisplayScaling(int* value) {
    char resp[CMDBUF_SIZE] = {'\0'};
    char cmdBuf[CMDBUF_SIZE] = {'\0'};
    int rc = -1;
    if (value == NULL) {
        ERROR("%s %d Error: pointers are NULL.\n",__FUNCTION__,__LINE__);
        return rc;
    }
    snprintf(cmdBuf, sizeof(cmdBuf)-1, "-r \"get scaling\"");
    rc = wstDisplaySendMessage(cmdBuf,resp);
    if (rc >= 0) {
        *value = getValueFromOutputString(resp, "scaling");
        DEBUG("%s %d get the scaling of graphic value %d",__FUNCTION__,__LINE__,*value);
    } else {
        ERROR("%s %d send message fail",__FUNCTION__,__LINE__);
    }
    return rc;
}

int getDisplayAutoFrmMode(int* value) {
    char resp[CMDBUF_SIZE] = {'\0'};
    char cmdBuf[CMDBUF_SIZE] = {'\0'};
    int rc = -1;
    if (value == NULL) {
        ERROR("%s %d Error: pointers are NULL.\n",__FUNCTION__,__LINE__);
        return rc;
    }
    snprintf(cmdBuf, sizeof(cmdBuf)-1, "-r \"get auto-frm-mode\"");
    rc = wstDisplaySendMessage(cmdBuf,resp);
    if (rc >= 0) {
        *value = getValueFromOutputString(resp, "auto-frm-mode");
        DEBUG("%s %d get auto-frm-mode %d",__FUNCTION__,__LINE__,*value);
    } else {
        ERROR("%s %d send message fail",__FUNCTION__,__LINE__);
    }
    return rc;
}

int setDisplayAutoFrmMode(int value) {
    int ret = -1;
    int rc = -1;
    char resp[CMDBUF_SIZE] = {'\0'};
    char cmdBuf[CMDBUF_SIZE] = {'\0'};
    DEBUG("%s %d weston set auto-frm-mode %d",__FUNCTION__,__LINE__, value);
    snprintf(cmdBuf, sizeof(cmdBuf)-1, "-r \"set auto-frm-mode %d\"",value);
    rc = wstDisplaySendMessage(cmdBuf,resp);
    if (rc >= 0) {
          ret = 0;
    } else {
        ERROR("%s %d send message fail",__FUNCTION__,__LINE__);
    }
    return ret;
}

int setDisplayScaling(int value) {
    int ret = -1;
    int rc = -1;
    char resp[CMDBUF_SIZE] = {'\0'};
    char cmdBuf[CMDBUF_SIZE] = {'\0'};
    DEBUG("%s %d weston set scaling value %d",__FUNCTION__,__LINE__, value);
    snprintf(cmdBuf, sizeof(cmdBuf)-1, "-r \"set scaling %d\"",value);
    rc = wstDisplaySendMessage(cmdBuf,resp);
    if (rc >= 0) {
          ret = 0;
    } else {
        ERROR("%s %d send message fail",__FUNCTION__,__LINE__);
    }
    return ret;
}


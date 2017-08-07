

#ifndef _OTA_LIB_C_
#define _OTA_LIB_C_

#include <stdio.h>
#include "iot_export_ota.h"
#include "iot_import_ota.h"
#include "ota_internal.h"


static const char *otalib_JsonValueOf(const char *json, uint32_t json_len, const char *key, uint32_t *val_len)
{
    int length;
    const char *val;     
    val = json_get_value_by_name((char *)json, json_len, (char *)key, &length, NULL);
    if (NULL != val) {
        *val_len = (uint32_t) length;
    }
    return val;
}

//Get the specific @key value, and copy to @dest
//0, successful; -1, failed
static int otalib_GetFirmwareFixlenPara(const char *json_doc,
                size_t json_doc_len,
                const char *key,
                char *dest,
                size_t dest_len)
{
    const char *pvalue;
    uint32_t val_len;

    if (NULL == (pvalue = otalib_JsonValueOf(json_doc, json_doc_len, key, &val_len))) {
        OTA_LOG_ERROR("Not '%s' key in json doc of OTA", key);
        return -1;
    }

    if (val_len > dest_len) {
        OTA_LOG_ERROR("value length of the key is too long");
        return -1;
    }

    memcpy(dest, pvalue, val_len);

    return 0;
}


//Get variant length parameter of firmware, and copy to @dest
//0, successful; -1, failed
static int otalib_GetFirmwareVarlenPara(const char *json_doc,
                size_t json_doc_len,
                const char *key,
                char **dest)
{
    const char *pvalue;
    uint32_t val_len;

    if (NULL == (pvalue = otalib_JsonValueOf(json_doc, json_doc_len, key, &val_len))) {
        OTA_LOG_ERROR("Not %s key in json doc of OTA", key);
        return -1;
    }

    if (NULL == (*dest = OTA_MALLOC(val_len + 1))) {
        OTA_LOG_ERROR("malloc failed");
        return -1;
    }

    memcpy(*dest, pvalue, val_len);
    (*dest)[val_len] = '\0';

    return 0;
}


int otalib_GetParams(const char *json_doc, uint32_t json_len, char **url, char **version, char *md5, uint32_t *file_size)
{
#define OTA_FILESIZE_STR_LEN    (16)
    char file_size_str[OTA_FILESIZE_STR_LEN + 1];

    //get version
    if (0 != otalib_GetFirmwareVarlenPara(json_doc, json_len, "version", version)) {
        OTA_LOG_ERROR("get value of version key failed");
        return -1;
    }

    //get URL
    if (0 != otalib_GetFirmwareVarlenPara(json_doc, json_len, "url", url)) {
        OTA_LOG_ERROR("get value of url key failed");
        return -1;
    }

    //get md5
    if (0 != otalib_GetFirmwareFixlenPara(json_doc, json_len, "md5", md5, 32)) {
        OTA_LOG_ERROR("get value of md5 key failed");
        return -1;
    }

    //get file size
    if (0 != otalib_GetFirmwareFixlenPara(json_doc, json_len, "size", file_size_str, OTA_FILESIZE_STR_LEN)) {
        OTA_LOG_ERROR("get value of size key failed");
        return -1;
    }
    file_size_str[OTA_FILESIZE_STR_LEN] = '\0';
    *file_size = atoi(file_size_str);

    return 0;

#undef OTA_FILESIZE_STR_LEN
}


//Generate firmware information according to @id, @version
//and then copy to @buf.
//0, successful; -1, failed
int otalib_GenInfoMsg(char *buf, size_t buf_len, uint32_t id, const char *version)
{
    int ret;
    ret = snprintf(buf,
            buf_len,
            "{\"id\":%d,\"params\":{\"version\":\"%s\"}}",
            id,
            version);

    if (ret < 0) {
        OTA_LOG_ERROR("snprintf failed");
        return -1;
    }

    return 0;
}


//Generate report information according to @id, @msg
//and then copy to @buf.
//0, successful; -1, failed
int otalib_GenReportMsg(char *buf, size_t buf_len, uint32_t id, int progress, const char *msg_detail)
{
    int ret;
    if (NULL == msg_detail) {
        ret = snprintf(buf,
                buf_len,
                "{\"id\":%d,\"params\":\{\"step\": \"%d\"}}",
                 id,
                 progress);
    } else {
        ret = snprintf(buf,
                buf_len,
                "{\"id\":%d,\"params\":\{\"step\": \"%d\",\"desc\":\"%s\"}}",
                 id,
                 progress,
                 msg_detail);
    }


    if (ret < 0) {
        OTA_LOG_ERROR("snprintf failed");
        return -1;
    } else if (ret >= buf_len) {
        OTA_LOG_ERROR("msg is too long");
        return IOT_OTAE_STR_TOO_LONG;
    }

    return 0;
}


#endif /* _OTA_LIB_C_ */
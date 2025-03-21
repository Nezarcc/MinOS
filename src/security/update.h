#ifndef SECURITY_UPDATE_H
#define SECURITY_UPDATE_H

#include <stdint.h>

/* Update status codes */
#define UPDATE_STATUS_NONE        0  /* No update available */
#define UPDATE_STATUS_AVAILABLE   1  /* Update available */
#define UPDATE_STATUS_DOWNLOADING 2  /* Update downloading */
#define UPDATE_STATUS_READY       3  /* Update ready to install */
#define UPDATE_STATUS_INSTALLING  4  /* Update installing */
#define UPDATE_STATUS_INSTALLED   5  /* Update installed */
#define UPDATE_STATUS_FAILED      6  /* Update failed */

/* Update package structure */
typedef struct {
    char name[64];           /* Package name */
    char version[32];        /* Package version */
    char description[256];   /* Package description */
    uint32_t size;           /* Package size in bytes */
    char hash[64];           /* Package hash */
    uint8_t is_security;     /* 1 if security update, 0 otherwise */
    uint8_t is_critical;     /* 1 if critical update, 0 otherwise */
} update_package_t;

/* Initialize the security update system */
void update_init(void);

/* Check for available updates */
int update_check(void);

/* Get the number of available updates */
int update_get_count(void);

/* Get an update package by index */
update_package_t* update_get_package(int index);

/* Download an update package */
int update_download(int index);

/* Install an update package */
int update_install(int index);

/* Install all available updates */
int update_install_all(void);

/* Get the current update status */
int update_get_status(void);

/* Verify the integrity of an update package */
int update_verify(int index);

/* Roll back the last update */
int update_rollback(void);

/* Set automatic update settings */
int update_set_auto(int enabled, int security_only);

/* Get automatic update settings */
int update_get_auto(int* security_only);

#endif /* SECURITY_UPDATE_H */

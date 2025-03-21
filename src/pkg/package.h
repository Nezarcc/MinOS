#ifndef PACKAGE_H
#define PACKAGE_H

#include <stdint.h>

/* Package dependency type */
#define DEP_TYPE_REQUIRED  0
#define DEP_TYPE_OPTIONAL  1
#define DEP_TYPE_CONFLICTS 2

/* Package dependency structure */
typedef struct {
    char name[64];       /* Package name */
    char version[32];    /* Version requirement */
    uint8_t type;        /* Dependency type */
} package_dependency_t;

/* Package structure */
typedef struct {
    char name[64];                   /* Package name */
    char version[32];                /* Package version */
    char description[256];           /* Package description */
    char maintainer[64];             /* Package maintainer */
    uint32_t size;                   /* Installed size in bytes */
    uint32_t download_size;          /* Download size in bytes */
    char hash[64];                   /* Package hash */
    uint8_t installed;               /* 1 if installed, 0 otherwise */
    package_dependency_t* deps;      /* Dependencies */
    uint32_t dep_count;              /* Number of dependencies */
    char** files;                    /* Files in package */
    uint32_t file_count;             /* Number of files */
} package_t;

/* Initialize the package management system */
void package_init(void);

/* Refresh the package database */
int package_refresh(void);

/* Search for packages */
package_t** package_search(const char* query, uint32_t* count);

/* Get package information */
package_t* package_get_info(const char* name);

/* Install a package */
int package_install(const char* name);

/* Remove a package */
int package_remove(const char* name);

/* Upgrade a package */
int package_upgrade(const char* name);

/* Upgrade all packages */
int package_upgrade_all(void);

/* Check if a package is installed */
int package_is_installed(const char* name);

/* Get the list of installed packages */
package_t** package_get_installed(uint32_t* count);

/* Verify the integrity of an installed package */
int package_verify(const char* name);

/* Add a package repository */
int package_add_repo(const char* url, const char* name, const char* key);

/* Remove a package repository */
int package_remove_repo(const char* name);

/* Get the list of package repositories */
char** package_get_repos(uint32_t* count);

/* Clean the package cache */
int package_clean_cache(void);

#endif /* PACKAGE_H */

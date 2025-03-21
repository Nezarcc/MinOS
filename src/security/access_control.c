#include "access_control.h"
#include "../kernel/kernel.h"
#include "user.h"
#include <stdint.h>
#include <string.h>

/* Initialize the access control system */
void access_control_init() {
    terminal_writestring("Initializing access control system...\n");
    
    // In a real implementation, this would initialize any global data structures
    
    terminal_writestring("Access control system initialized\n");
}

/* Create a new access control list */
access_control_list_t* access_control_create() {
    // Allocate the ACL structure
    access_control_list_t* acl = (access_control_list_t*)kmalloc(sizeof(access_control_list_t));
    if (!acl) {
        return NULL;
    }
    
    // Initialize with a default capacity
    acl->capacity = 4;
    acl->entry_count = 0;
    
    // Allocate the entries array
    acl->entries = (access_control_entry_t*)kmalloc(acl->capacity * sizeof(access_control_entry_t));
    if (!acl->entries) {
        kfree(acl);
        return NULL;
    }
    
    return acl;
}

/* Free an access control list */
void access_control_free(access_control_list_t* acl) {
    if (!acl) {
        return;
    }
    
    // Free the entries array
    if (acl->entries) {
        kfree(acl->entries);
    }
    
    // Free the ACL structure
    kfree(acl);
}

/* Add an entry to an access control list */
int access_control_add_entry(access_control_list_t* acl, uint8_t type, uint32_t subject_id, uint8_t is_group, uint16_t perms) {
    if (!acl) {
        return -1;
    }
    
    // Check if we need to resize the entries array
    if (acl->entry_count >= acl->capacity) {
        // Double the capacity
        uint32_t new_capacity = acl->capacity * 2;
        
        // Allocate a new entries array
        access_control_entry_t* new_entries = (access_control_entry_t*)kmalloc(new_capacity * sizeof(access_control_entry_t));
        if (!new_entries) {
            return -1;
        }
        
        // Copy the existing entries
        memcpy(new_entries, acl->entries, acl->entry_count * sizeof(access_control_entry_t));
        
        // Free the old entries array
        kfree(acl->entries);
        
        // Update the ACL
        acl->entries = new_entries;
        acl->capacity = new_capacity;
    }
    
    // Add the new entry
    acl->entries[acl->entry_count].type = type;
    acl->entries[acl->entry_count].subject_id = subject_id;
    acl->entries[acl->entry_count].is_group = is_group;
    acl->entries[acl->entry_count].perms = perms;
    
    acl->entry_count++;
    
    return 0;
}

/* Remove an entry from an access control list */
int access_control_remove_entry(access_control_list_t* acl, uint32_t index) {
    if (!acl || index >= acl->entry_count) {
        return -1;
    }
    
    // Shift all entries after the removed one
    for (uint32_t i = index; i < acl->entry_count - 1; i++) {
        acl->entries[i] = acl->entries[i + 1];
    }
    
    acl->entry_count--;
    
    return 0;
}

/* Check if a user has permission to access a resource */
int access_control_check_permission(access_control_list_t* acl, uint32_t uid, uint32_t* groups, uint32_t group_count, uint16_t requested_perms) {
    if (!acl) {
        return 0; // No ACL means no access
    }
    
    // Start with no permissions
    uint16_t allowed_perms = 0;
    uint16_t denied_perms = 0;
    
    // Process all entries in the ACL
    for (uint32_t i = 0; i < acl->entry_count; i++) {
        access_control_entry_t* entry = &acl->entries[i];
        
        // Check if this entry applies to the user
        int applies = 0;
        
        if (entry->is_group) {
            // Check if the user is in this group
            for (uint32_t j = 0; j < group_count; j++) {
                if (groups[j] == entry->subject_id) {
                    applies = 1;
                    break;
                }
            }
        } else {
            // Check if this is the user
            if (uid == entry->subject_id) {
                applies = 1;
            }
        }
        
        // If the entry applies, update permissions
        if (applies) {
            if (entry->type == ACE_TYPE_ALLOW) {
                allowed_perms |= entry->perms;
            } else if (entry->type == ACE_TYPE_DENY) {
                denied_perms |= entry->perms;
            }
        }
    }
    
    // Deny takes precedence over allow
    uint16_t effective_perms = allowed_perms & ~denied_perms;
    
    // Check if all requested permissions are granted
    return (effective_perms & requested_perms) == requested_perms;
}

/* Convert traditional Unix permissions to an access control list */
access_control_list_t* access_control_from_unix_perms(uint16_t mode, uint32_t owner_uid, uint32_t group_gid) {
    // Create a new ACL
    access_control_list_t* acl = access_control_create();
    if (!acl) {
        return NULL;
    }
    
    // Add owner permissions
    uint16_t owner_perms = 0;
    if (mode & 0400) owner_perms |= PERM_READ;
    if (mode & 0200) owner_perms |= PERM_WRITE;
    if (mode & 0100) owner_perms |= PERM_EXECUTE;
    
    access_control_add_entry(acl, ACE_TYPE_ALLOW, owner_uid, 0, owner_perms);
    
    // Add group permissions
    uint16_t group_perms = 0;
    if (mode & 0040) group_perms |= PERM_READ;
    if (mode & 0020) group_perms |= PERM_WRITE;
    if (mode & 0010) group_perms |= PERM_EXECUTE;
    
    access_control_add_entry(acl, ACE_TYPE_ALLOW, group_gid, 1, group_perms);
    
    // Add other permissions
    uint16_t other_perms = 0;
    if (mode & 0004) other_perms |= PERM_READ;
    if (mode & 0002) other_perms |= PERM_WRITE;
    if (mode & 0001) other_perms |= PERM_EXECUTE;
    
    // In a real implementation, there would be a special "everyone" ID
    // For now, we'll use a placeholder
    uint32_t everyone_id = 0xFFFFFFFF;
    access_control_add_entry(acl, ACE_TYPE_ALLOW, everyone_id, 1, other_perms);
    
    // Add special permissions
    if (mode & PERM_SETUID) {
        // In a real implementation, this would add a special ACE for setuid
    }
    
    if (mode & PERM_SETGID) {
        // In a real implementation, this would add a special ACE for setgid
    }
    
    if (mode & PERM_STICKY) {
        // In a real implementation, this would add a special ACE for sticky bit
    }
    
    return acl;
}

/* Convert an access control list to traditional Unix permissions */
uint16_t access_control_to_unix_perms(access_control_list_t* acl, uint32_t* owner_uid, uint32_t* group_gid) {
    if (!acl || !owner_uid || !group_gid) {
        return 0;
    }
    
    // Default permissions
    uint16_t mode = 0;
    
    // Find the owner entry
    for (uint32_t i = 0; i < acl->entry_count; i++) {
        access_control_entry_t* entry = &acl->entries[i];
        
        if (!entry->is_group && entry->type == ACE_TYPE_ALLOW) {
            // This is a user allow entry
            *owner_uid = entry->subject_id;
            
            // Set owner permissions
            if (entry->perms & PERM_READ) mode |= 0400;
            if (entry->perms & PERM_WRITE) mode |= 0200;
            if (entry->perms & PERM_EXECUTE) mode |= 0100;
            
            break;
        }
    }
    
    // Find the group entry
    for (uint32_t i = 0; i < acl->entry_count; i++) {
        access_control_entry_t* entry = &acl->entries[i];
        
        if (entry->is_group && entry->type == ACE_TYPE_ALLOW && entry->subject_id != 0xFFFFFFFF) {
            // This is a group allow entry
            *group_gid = entry->subject_id;
            
            // Set group permissions
            if (entry->perms & PERM_READ) mode |= 0040;
            if (entry->perms & PERM_WRITE) mode |= 0020;
            if (entry->perms & PERM_EXECUTE) mode |= 0010;
            
            break;
        }
    }
    
    // Find the "everyone" entry
    for (uint32_t i = 0; i < acl->entry_count; i++) {
        access_control_entry_t* entry = &acl->entries[i];
        
        if (entry->is_group && entry->type == ACE_TYPE_ALLOW && entry->subject_id == 0xFFFFFFFF) {
            // This is the everyone allow entry
            
            // Set other permissions
            if (entry->perms & PERM_READ) mode |= 0004;
            if (entry->perms & PERM_WRITE) mode |= 0002;
            if (entry->perms & PERM_EXECUTE) mode |= 0001;
            
            break;
        }
    }
    
    // In a real implementation, this would also handle special permissions
    
    return mode;
}

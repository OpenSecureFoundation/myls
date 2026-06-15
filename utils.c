#define _POSIX_C_SOURCE 200809L
#include "utils.h"
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/xattr.h>

#ifndef S_ISVTX
# define S_ISVTX 01000
#endif

void format_permissions(mode_t mode, char *str)
{
	int	i;

	i = 0;
	while (i < 10)
		str[i++] = '-';
	str[10] = '\0';
	if (S_ISDIR(mode))
		str[0] = 'd';
	else if (S_ISLNK(mode))
		str[0] = 'l';
	else if (S_ISBLK(mode))
		str[0] = 'b';
	else if (S_ISCHR(mode))
		str[0] = 'c';
	else if (S_ISFIFO(mode))
		str[0] = 'p';
	else if (S_ISSOCK(mode))
		str[0] = 's';
	if (mode & S_IRUSR)
		str[1] = 'r';
	if (mode & S_IWUSR)
		str[2] = 'w';
	if (mode & S_IXUSR)
		str[3] = 'x';
	if (mode & S_IRGRP)
		str[4] = 'r';
	if (mode & S_IWGRP)
		str[5] = 'w';
	if (mode & S_IXGRP)
		str[6] = 'x';
	if (mode & S_IROTH)
		str[7] = 'r';
	if (mode & S_IWOTH)
		str[8] = 'w';
	if (mode & S_IXOTH)
		str[9] = 'x';
	if (mode & S_ISUID)
		str[3] = (mode & S_IXUSR) ? 's' : 'S';
	if (mode & S_ISGID)
		str[6] = (mode & S_IXGRP) ? 's' : 'S';
	if (mode & S_ISVTX)
		str[9] = (mode & S_IXOTH) ? 't' : 'T';
}

static int	is_no_xattr_error(int err)
{
	return err == ENODATA || err == ENOTSUP || err == EOPNOTSUPP;
}

static int	entry_xattr_status(const t_entry *entry, const char *name)
{
	ssize_t	size;

	errno = 0;
	if (entry->stat_followed)
		size = getxattr(entry->path, name, NULL, 0);
	else
		size = lgetxattr(entry->path, name, NULL, 0);
	if (size >= 0)
		return 1;
	if (is_no_xattr_error(errno))
		return 0;
	return -1;
}

static char	access_method_marker(const t_entry *entry)
{
	int	acl;
	int	selinux;
	int	capability;

	acl = entry_xattr_status(entry, "system.posix_acl_access");
	if (acl > 0)
		return '+';
	selinux = entry_xattr_status(entry, "security.selinux");
	capability = entry_xattr_status(entry, "security.capability");
	if (acl < 0 || selinux < 0 || capability < 0)
		return '?';
	if (selinux > 0 || capability > 0)
		return '.';
	return ' ';
}

void format_permissions_entry(const t_entry *entry, char *str)
{
	char	marker;

	format_permissions(entry->info.st_mode, str);
	marker = access_method_marker(entry);
	if (marker != ' ') {
		str[10] = marker;
		str[11] = '\0';
	}
}

time_t entry_selected_time(const t_entry *entry, const t_options *options)
{
	if (options->option_u)
		return entry->info.st_atime;
	if (options->option_c)
		return entry->info.st_ctime;
	return entry->info.st_mtime;
}

long long block_unit(const t_options *options)
{
	if (options->option_block_size_value > 0)
		return options->option_block_size_value;
	if (options->option_k)
		return 1024;
	return 1024;
}

long long ceil_div_ll(long long value, long long divisor)
{
	if (divisor <= 0)
		return value;
	if (value <= 0)
		return 0;
	return (value + divisor - 1) / divisor;
}

static void format_human(double value, int base, char *str, size_t size_buf)
{
	const char	*units_1024[] = {"B", "K", "M", "G", "T", "P"};
	const char	*units_1000[] = {"B", "k", "M", "G", "T", "P"};
	const char	**units;
	int			i;

	units = (base == 1000) ? units_1000 : units_1024;
	i = 0;
	while (value >= base && i < 5) {
		value /= base;
		i++;
	}
	if (i == 0)
		snprintf(str, size_buf, "%.0f", value);
	else if (value >= 10)
		snprintf(str, size_buf, "%.0f%s", value, units[i]);
	else
		snprintf(str, size_buf, "%.1f%s", value, units[i]);
}

void format_file_size(off_t size, const t_options *options,
	char *str, size_t size_buf)
{
	if (options->option_h)
		format_human((double)size, 1024, str, size_buf);
	else if (options->option_si)
		format_human((double)size, 1000, str, size_buf);
	else if (options->option_block_size_value > 0)
		snprintf(str, size_buf, "%lld",
			ceil_div_ll((long long)size, options->option_block_size_value));
	else
		snprintf(str, size_buf, "%lld", (long long)size);
}

void format_alloc_size(blkcnt_t blocks, const t_options *options,
	char *str, size_t size_buf)
{
	long long	bytes;
	long long	unit;

	bytes = (long long)blocks * 512;
	if (options->option_h)
		format_human((double)bytes, 1024, str, size_buf);
	else if (options->option_si)
		format_human((double)bytes, 1000, str, size_buf);
	else {
		unit = block_unit(options);
		snprintf(str, size_buf, "%lld", ceil_div_ll(bytes, unit));
	}
}

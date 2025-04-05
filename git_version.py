Import("env")
import subprocess
import os


def get_git_version():
    try:
        # Get the most recent tag
        tag = subprocess.check_output(
            ["git", "describe", "--tags", "--abbrev=0"], universal_newlines=True
        ).strip()

        # Check if there are changes since the tag
        dirty_check = subprocess.call(
            ["git", "diff", "--quiet", "HEAD"], stderr=subprocess.DEVNULL
        )

        # Check if there are commits since the tag
        commits_since_tag = subprocess.check_output(
            ["git", "rev-list", f"{tag}..HEAD", "--count"], universal_newlines=True
        ).strip()

        # Add -dirty suffix if there are uncommitted changes or commits since tag
        if dirty_check != 0 or int(commits_since_tag) > 0:
            tag += "-dirty"

        return tag
    except Exception as e:
        # Default version if git command fails
        print(f"Git command failed: {e}")
        return "v0.0.0-unknown"


# Define a new build flag with the git version
version = get_git_version()
print(f"Building version: {version}")
env.Append(CPPDEFINES=[("FIRMWARE_VERSION", f'\\"{version}\\"')])

// SPDX-License-Identifier: GPL-3.0-only
#pragma once

#include "modplatform/ModIndex.h"

namespace OptimizationSafety {
struct InstalledOptimizationMod {
    QString projectId;
    QString versionId;
    bool enabled = false;
    QList<ModPlatform::Dependency> dependencies;
};

inline bool candidateFits(const ModPlatform::IndexedVersion& candidate, const QList<InstalledOptimizationMod>& installed)
{
    const QString id = candidate.addonId.toString();
    for (const auto& mod : installed) {
        // A disabled copy is still a duplicate; never overwrite or silently enable it.
        if (mod.projectId == id)
            return false;
        if (!mod.enabled)
            continue;
        for (const auto& dep : mod.dependencies) {
            if (dep.type == ModPlatform::DependencyType::UNKNOWN)
                return false;
            if (dep.addonId.toString() == id && dep.type == ModPlatform::DependencyType::INCOMPATIBLE &&
                (dep.version.isEmpty() || dep.version == candidate.fileId.toString()))
                return false;
            if (dep.addonId.toString() == id && dep.type == ModPlatform::DependencyType::REQUIRED &&
                !dep.version.isEmpty() && dep.version != candidate.fileId.toString())
                return false;
        }
    }
    for (const auto& dep : candidate.dependencies) {
        if (dep.type == ModPlatform::DependencyType::UNKNOWN)
            return false;
        bool found = false;
        for (const auto& mod : installed) {
            if (mod.enabled && mod.projectId == dep.addonId.toString() &&
                (dep.version.isEmpty() || mod.versionId == dep.version)) {
                found = true;
                break;
            }
        }
        if (dep.type == ModPlatform::DependencyType::REQUIRED && !found)
            return false;
        if (dep.type == ModPlatform::DependencyType::INCOMPATIBLE && found)
            return false;
    }
    return true;
}
} // namespace OptimizationSafety

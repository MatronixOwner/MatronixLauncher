// SPDX-License-Identifier: GPL-3.0-only
/*
 *  Prism Launcher - Minecraft Launcher
 *  Copyright (c) 2022 Jamie Mansfield <jmansfield@cadixdev.org>
 *  Copyright (C) 2022 Sefa Eyeoglu <contact@scrumplex.net>
 *  Copyright (C) 2022 TheKodeToad <TheKodeToad@proton.me>
 *  Copyright (c) 2023 Trial97 <alexandru.tripon97@gmail.com>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, version 3.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * This file incorporates work covered by the following copyright and
 * permission notice:
 *
 *      Copyright 2013-2021 MultiMC Contributors
 *
 *      Licensed under the Apache License, Version 2.0 (the "License");
 *      you may not use this file except in compliance with the License.
 *      You may obtain a copy of the License at
 *
 *          http://www.apache.org/licenses/LICENSE-2.0
 *
 *      Unless required by applicable law or agreed to in writing, software
 *      distributed under the License is distributed on an "AS IS" BASIS,
 *      WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *      See the License for the specific language governing permissions and
 *      limitations under the License.
 */

#include "ModFolderPage.h"
#include "minecraft/mod/Resource.h"
#include "ui/dialogs/ExportToModListDialog.h"
#include "ui/dialogs/InstallLoaderDialog.h"
#include "ui_ExternalResourcesPage.h"

#include <QAbstractItemModel>
#include <QAction>
#include <QCryptographicHash>
#include <QEvent>
#include <QFile>
#include <QKeyEvent>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMenu>
#include <QMessageBox>
#include <QSet>
#include <QSortFilterProxyModel>
#include <QUrlQuery>
#include <algorithm>
#include <memory>

#include "Application.h"
#include "BuildConfig.h"
#include "Json.h"
#include "ResourceDownloadTask.h"
#include "modplatform/modrinth/ModrinthAPI.h"
#include "net/ApiDownload.h"
#include "net/NetJob.h"

#include "ui/dialogs/CustomMessageBox.h"
#include "ui/dialogs/ResourceDownloadDialog.h"
#include "ui/dialogs/ResourceUpdateDialog.h"

#include "minecraft/PackProfile.h"
#include "minecraft/VersionFilterData.h"
#include "minecraft/mod/Mod.h"
#include "minecraft/mod/ModFolderModel.h"

#include "tasks/ConcurrentTask.h"
#include "tasks/Task.h"
#include "ui/dialogs/ProgressDialog.h"

ModFolderPage::ModFolderPage(BaseInstance* inst, ModFolderModel* model, QWidget* parent)
    : ExternalResourcesPage(inst, model, parent), m_model(model)
{
    ui->actionDownloadItem->setText(tr("Download Mods"));
    ui->actionDownloadItem->setToolTip(tr("Download mods from online mod platforms"));
    ui->actionDownloadItem->setEnabled(true);
    ui->actionsToolbar->insertActionBefore(ui->actionAddItem, ui->actionDownloadItem);

    connect(ui->actionDownloadItem, &QAction::triggered, this, &ModFolderPage::downloadMods);

    auto* compatibility = new QAction(tr("Check Compatibility"), this);
    compatibility->setToolTip(tr("Check enabled mods using locally available metadata; no files will be changed."));
    ui->actionsToolbar->insertActionBefore(ui->actionAddItem, compatibility);
    connect(compatibility, &QAction::triggered, this, &ModFolderPage::checkCompatibility);

    auto* fix = new QAction(tr("Auto-fix"), this);
    fix->setToolTip(tr("Inspect the instance and review safe fixes before applying them."));
    ui->actionsToolbar->insertActionBefore(ui->actionAddItem, fix);
    connect(fix, &QAction::triggered, this, &ModFolderPage::autoFix);

    auto* optimize = new QAction(tr("Optimization"), this);
    optimize->setToolTip(tr("Find a compatible optimization mod on Modrinth and review it before installation."));
    ui->actionsToolbar->insertActionBefore(ui->actionAddItem, optimize);
    connect(optimize, &QAction::triggered, this, &ModFolderPage::optimizeMods);

    ui->actionUpdateItem->setToolTip(tr("Try to check or update all selected mods (all mods if none are selected)"));
    connect(ui->actionUpdateItem, &QAction::triggered, this, &ModFolderPage::updateMods);
    ui->actionsToolbar->insertActionBefore(ui->actionAddItem, ui->actionUpdateItem);

    auto* updateMenu = new QMenu(this);

    auto* update = updateMenu->addAction(tr("Check for Updates"));
    connect(update, &QAction::triggered, this, &ModFolderPage::updateMods);

    updateMenu->addAction(ui->actionVerifyItemDependencies);
    connect(ui->actionVerifyItemDependencies, &QAction::triggered, this, [this] { updateMods(true); });

    auto depsDisabled = APPLICATION->settings()->getSetting("ModDependenciesDisabled");
    ui->actionVerifyItemDependencies->setVisible(!depsDisabled->get().toBool());
    connect(depsDisabled.get(), &Setting::SettingChanged, this,
            [this](const Setting&, const QVariant& value) { ui->actionVerifyItemDependencies->setVisible(!value.toBool()); });

    updateMenu->addAction(ui->actionResetItemMetadata);
    connect(ui->actionResetItemMetadata, &QAction::triggered, this, &ModFolderPage::deleteModMetadata);

    ui->actionUpdateItem->setMenu(updateMenu);

    ui->actionChangeVersion->setToolTip(tr("Change a mod's version."));
    connect(ui->actionChangeVersion, &QAction::triggered, this, &ModFolderPage::changeModVersion);
    ui->actionsToolbar->insertActionAfter(ui->actionUpdateItem, ui->actionChangeVersion);

    ui->actionViewHomepage->setToolTip(tr("View the homepages of all selected mods."));

    ui->actionExportMetadata->setToolTip(tr("Export mod's metadata to text."));
    connect(ui->actionExportMetadata, &QAction::triggered, this, &ModFolderPage::exportModMetadata);
    ui->actionsToolbar->insertActionAfter(ui->actionViewHomepage, ui->actionExportMetadata);

    ui->actionsToolbar->insertActionAfter(ui->actionViewFolder, ui->actionViewConfigs);
}

bool ModFolderPage::shouldDisplay() const
{
    return true;
}

void ModFolderPage::updateFrame(const QModelIndex& current, [[maybe_unused]] const QModelIndex& previous)
{
    auto sourceCurrent = m_filterModel->mapToSource(current);
    int row = sourceCurrent.row();
    const Mod& mod = m_model->at(row);
    ui->frame->updateWithMod(mod);
}

void ModFolderPage::removeItems(const QItemSelection& selection)
{
    if (m_instance != nullptr && m_instance->isRunning()) {
        auto response = CustomMessageBox::selectable(this, tr("Confirm Delete"),
                                                     tr("If you remove mods while the game is running it may crash your game.\n"
                                                        "Are you sure you want to do this?"),
                                                     QMessageBox::Warning, QMessageBox::Yes | QMessageBox::No, QMessageBox::No)
                            ->exec();

        if (response != QMessageBox::Yes) {
            return;
        }
    }

    auto indexes = selection.indexes();
    auto affected = m_model->getAffectedMods(indexes, EnableAction::DISABLE);
    if (!affected.isEmpty()) {
        auto response = CustomMessageBox::selectable(this, tr("Confirm Disable"),
                                                     tr("The mods you are trying to delete are required by %1 mods.\n"
                                                        "Do you want to disable them?")
                                                         .arg(affected.length()),
                                                     QMessageBox::Warning, QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel,
                                                     QMessageBox::Cancel)
                            ->exec();

        if (response == QMessageBox::Cancel) {
            return;
        }
        if (response == QMessageBox::Yes) {
            m_model->setResourceEnabled(affected, EnableAction::DISABLE);
        }
    }
    m_model->deleteResources(indexes);
}

void ModFolderPage::checkCompatibility()
{
    if (!m_instance || m_instance->typeName() != "Minecraft") {
        QMessageBox::information(this, tr("Compatibility check"), tr("This check is only available for Minecraft instances."));
        return;
    }

    auto* profile = static_cast<MinecraftInstance*>(m_instance)->getPackProfile();
    auto minecraft = profile ? profile->getComponent("net.minecraft") : nullptr;
    if (!minecraft) {
        QMessageBox::warning(this, tr("Compatibility check"), tr("The Minecraft version could not be determined."));
        return;
    }

    const auto gameVersion = minecraft->getVersion();
    const auto supportedLoaders = profile->getSupportedModLoaders();
    const auto mods = m_model->allMods();
    QSet<QString> enabledIds;
    QSet<QString> duplicateIds;
    for (auto* mod : mods) {
        if (mod->enabled() && !mod->mod_id().isEmpty()) {
            const auto id = mod->mod_id().toLower();
            if (enabledIds.contains(id))
                duplicateIds.insert(id);
            enabledIds.insert(id);
        }
    }

    QStringList warnings;
    for (const auto& id : duplicateIds)
        warnings << tr("Multiple enabled mods declare the same ID: %1.").arg(id);
    int unknownVersions = 0;
    int unknownLoaders = 0;
    for (auto* mod : mods) {
        if (!mod->enabled())
            continue;
        const auto versions = mod->mcVersions();
        if (versions.isEmpty()) {
            ++unknownVersions;
        } else if (!versions.contains(gameVersion, Qt::CaseInsensitive)) {
            warnings << tr("%1: Minecraft %2 is not listed in its metadata (listed: %3).")
                            .arg(mod->name(), gameVersion, versions.join(", "));
        }
        const auto metadata = mod->metadata();
        if (!metadata || metadata->loaders == ModPlatform::ModLoaderType::None) {
            ++unknownLoaders;
        } else if (!supportedLoaders || !(metadata->loaders & *supportedLoaders)) {
            warnings << tr("%1: its listed loaders (%2) do not match this instance.").arg(mod->name(), mod->loaders());
        }
        for (const auto& dependency : mod->dependencies()) {
            if (!dependency.isEmpty() && !enabledIds.contains(dependency.toLower())) {
                warnings << tr("%1: required mod '%2' was not found among enabled mod IDs.").arg(mod->name(), dependency);
            }
        }
        if (metadata) {
            for (const auto& dependency : metadata->dependencies) {
                if (dependency.addonId.toString().isEmpty())
                    continue;
                bool found = false;
                for (auto* other : mods) {
                    const auto otherMeta = other->metadata();
                    if (other->enabled() && otherMeta && otherMeta->provider == metadata->provider &&
                        otherMeta->project_id == dependency.addonId) {
                        found = true;
                        break;
                    }
                }
                if (!found && dependency.type == ModPlatform::DependencyType::REQUIRED)
                    warnings << tr("%1: required project %2 was not found in enabled indexed mods; it may be embedded or unindexed.")
                                    .arg(mod->name(), dependency.addonId.toString());
                if (found && dependency.type == ModPlatform::DependencyType::INCOMPATIBLE)
                    warnings << tr("%1: project %2 is listed as incompatible and appears to be enabled.")
                                    .arg(mod->name(), dependency.addonId.toString());
            }
        }
    }

    QMessageBox box(this);
    box.setWindowTitle(tr("Compatibility check"));
    box.setIcon(warnings.isEmpty() ? QMessageBox::Information : QMessageBox::Warning);
    box.setText(warnings.isEmpty() ? tr("No problems were detected in the available metadata.")
                                  : tr("Possible problems were found (%1). See details before making changes.").arg(warnings.size()));
    warnings << tr("Minecraft version metadata unavailable for %1 enabled mods; loader metadata unavailable for %2.")
                    .arg(unknownVersions).arg(unknownLoaders);
    warnings << tr("This offline check is incomplete: metadata may be missing or outdated, embedded dependencies may not be detected, "
                   "and dependency version constraints and unindexed conflicts are not checked. No files were changed.");
    box.setDetailedText(warnings.join('\n'));
    box.exec();
}

namespace {
QByteArray fileDigest(const QString& path)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly))
        return {};
    QCryptographicHash hash(QCryptographicHash::Sha256);
    return hash.addData(&file) ? hash.result() : QByteArray{};
}
}

void ModFolderPage::autoFix()
{
    if (!m_instance || m_instance->typeName() != "Minecraft" || m_instance->isRunning()) {
        QMessageBox::warning(this, tr("Auto-fix"), tr("Close the game before checking and fixing a Minecraft instance."));
        return;
    }

    // Only disable byte-identical duplicates; never infer which different version to keep.
    QHash<QByteArray, QString> originals;
    QList<QPair<QString, QString>> duplicates;
    for (auto* mod : m_model->allMods()) {
        const auto info = mod->fileinfo();
        if (!mod->enabled() || !info.isFile() || info.isSymLink())
            continue;
        const auto digest = fileDigest(info.absoluteFilePath());
        if (digest.isEmpty())
            continue;
        if (originals.contains(digest))
            duplicates.append({ originals.value(digest), info.absoluteFilePath() });
        else
            originals.insert(digest, info.absoluteFilePath());
    }

    QStringList problems;
    for (const auto& pair : duplicates)
        problems << tr("Identical enabled files: %1 and %2").arg(QFileInfo(pair.first).fileName(), QFileInfo(pair.second).fileName());
    if (problems.isEmpty()) {
        QMessageBox::information(this, tr("Auto-fix: inspection"),
                                 tr("No byte-identical enabled mod files were found. Other problems may require manual repair. "
                                    "Use the compatibility check to see additional warnings. No files were changed."));
        return;
    }
    QMessageBox report(this);
    report.setWindowTitle(tr("Auto-fix: inspection"));
    report.setIcon(QMessageBox::Warning);
    report.setText(tr("Found %1 duplicate file(s). Other problems may require manual repair.").arg(problems.size()));
    report.setDetailedText(problems.join('\n'));
    report.exec();

    QStringList changes;
    for (const auto& pair : duplicates)
        changes << tr("Disable %1 (keep %2)").arg(QFileInfo(pair.second).fileName(), QFileInfo(pair.first).fileName());
    if (QMessageBox::question(this, tr("Confirm auto-fix"),
                              tr("The following files will be renamed with a .disabled suffix; no files will be deleted:\n\n%1\n\nApply these changes?")
                                  .arg(changes.join('\n')),
                              QMessageBox::Yes | QMessageBox::No, QMessageBox::No) != QMessageBox::Yes)
        return;

    if (m_instance->isRunning()) {
        QMessageBox::warning(this, tr("Auto-fix"), tr("The game started; no changes were made."));
        return;
    }
    // Revalidate after the confirmation dialog: the directory may have changed.
    QModelIndexList toDisable;
    const auto current = m_model->allMods();
    for (const auto& pair : duplicates) {
        const auto first = fileDigest(pair.first);
        if (first.isEmpty() || first != fileDigest(pair.second) || QFileInfo::exists(pair.second + ".disabled"))
            continue;
        for (int row = 0; row < current.size(); ++row) {
            auto* mod = current.at(row);
            if (mod->enabled() && !mod->fileinfo().isSymLink() && mod->fileinfo().absoluteFilePath() == pair.second) {
                toDisable << m_model->index(row, 0);
                break;
            }
        }
    }
    if (toDisable.isEmpty()) {
        QMessageBox::warning(this, tr("Auto-fix"), tr("Files changed during inspection; no fixes were applied. Check again."));
        return;
    }
    const bool success = m_model->ResourceFolderModel::setResourceEnabled(toDisable, EnableAction::DISABLE);
    m_model->update();
    QMessageBox::information(this, tr("Auto-fix"), success && toDisable.size() == duplicates.size()
        ? tr("The selected duplicate files were disabled. Other problems still need to be checked manually.")
        : tr("Some fixes were skipped or failed because files changed or could not be renamed. Check the mod list."));
}

void ModFolderPage::optimizeMods()
{
    if (!m_instance || m_instance->typeName() != "Minecraft" || m_instance->isRunning()) {
        QMessageBox::warning(this, tr("Optimization"), tr("Close the game before installing optimization mods."));
        return;
    }
    auto* profile = static_cast<MinecraftInstance*>(m_instance)->getPackProfile();
    auto minecraft = profile ? profile->getComponent("net.minecraft") : nullptr;
    const auto loaders = profile ? profile->getModLoaders() : std::nullopt;
    if (!minecraft || !loaders || !ModPlatform::hasSingleModLoaderSelected(*loaders)) {
        QMessageBox::warning(this, tr("Optimization"), tr("Exactly one supported mod loader and a Minecraft version are required."));
        return;
    }
    // Unknown local mods cannot be reliably checked for conflicts. Fail closed rather than guess.
    if (!m_model->allMods().isEmpty()) {
        QMessageBox::information(this, tr("Optimization"),
                                 tr("Automatic selection is currently limited to an empty mods folder. "
                                    "Existing mods, including disabled ones, cannot yet be checked reliably for conflicts; nothing was installed."));
        return;
    }
    QString slug;
    if (*loaders == ModPlatform::Fabric || *loaders == ModPlatform::Quilt)
        slug = "sodium";
    else if (*loaders == ModPlatform::Forge || *loaders == ModPlatform::NeoForge)
        slug = "embeddium";
    else {
        QMessageBox::information(this, tr("Optimization"), tr("No conservative optimization recommendation is available for this loader."));
        return;
    }
    const QString game = minecraft->getVersion();
    const QString loader = ModPlatform::getModLoaderAsString(ModPlatform::modLoaderTypesToList(*loaders).first());
    QUrl versionUrl(BuildConfig.MODRINTH_PROD_URL + "/project/" + slug + "/version");
    QUrlQuery query;
    query.addQueryItem("game_versions", QString::fromUtf8(QJsonDocument(QJsonArray{ game }).toJson(QJsonDocument::Compact)));
    query.addQueryItem("loaders", QString::fromUtf8(QJsonDocument(QJsonArray{ loader }).toJson(QJsonDocument::Compact)));
    versionUrl.setQuery(query);
    auto job = makeShared<NetJob>(tr("Finding optimization mods"), APPLICATION->network());
    auto [projectAction, projectData] = Net::ApiDownload::makeByteArray(QUrl(BuildConfig.MODRINTH_PROD_URL + "/project/" + slug));
    auto [versionAction, versionData] = Net::ApiDownload::makeByteArray(versionUrl);
    job->addNetAction(projectAction);
    job->addNetAction(versionAction);
    ProgressDialog progress(this);
    progress.setSkipButton(true, tr("Abort"));
    if (progress.execWithTask(job.get()) != QDialog::Accepted || !job->wasSuccessful()) {
        QMessageBox::warning(this, tr("Optimization"), tr("Could not retrieve recommendations from Modrinth. No files were changed."));
        return;
    }
    QJsonParseError projectError{}, versionError{};
    auto project = QJsonDocument::fromJson(*projectData, &projectError);
    auto versions = QJsonDocument::fromJson(*versionData, &versionError);
    if (projectError.error != QJsonParseError::NoError || versionError.error != QJsonParseError::NoError || !project.isObject() || !versions.isArray()) {
        QMessageBox::warning(this, tr("Optimization"), tr("Modrinth returned invalid data. No files were changed."));
        return;
    }
    auto pack = std::make_shared<ModPlatform::IndexedPack>();
    ModPlatform::IndexedVersion selected;
    try {
        auto obj = project.object();
        Modrinth::loadIndexedPack(*pack, obj);
        for (const auto& entry : versions.array()) {
            if (!entry.isObject())
                continue;
            auto versionObj = entry.toObject();
            auto candidate = Modrinth::loadIndexedPackVersion(versionObj);
            bool unresolved = false;
            for (const auto& dep : candidate.dependencies) {
                if (dep.type == ModPlatform::DependencyType::REQUIRED || dep.type == ModPlatform::DependencyType::UNKNOWN)
                    unresolved = true;
            }
            if (!unresolved && candidate.addonId == pack->addonId && candidate.mcVersion.contains(game) &&
                (candidate.loaders & *loaders) && !candidate.hash.isEmpty() && !candidate.downloadUrl.isEmpty() &&
                !candidate.fileName.isEmpty() && QFileInfo(candidate.fileName).fileName() == candidate.fileName) {
                selected = candidate;
                break;
            }
        }
    } catch (const Json::JsonException&) {
        QMessageBox::warning(this, tr("Optimization"), tr("Modrinth returned incomplete data. No files were changed."));
        return;
    }
    if (selected.fileName.isEmpty()) {
        QMessageBox::information(this, tr("Optimization"), tr("No compatible self-contained recommendation was found for Minecraft %1 and %2. No files were changed.").arg(game, loader));
        return;
    }
    const QString message = tr("Recommended Modrinth mod for Minecraft %1 / %2:\n\n%3 — %4\nFile: %5\n\n"
                               "Only this file will be installed. Compatibility data can be incomplete; keep a backup of your instance. Install it?")
                                .arg(game, loader, pack->name, selected.version, selected.fileName);
    if (QMessageBox::question(this, tr("Confirm optimization"), message, QMessageBox::Yes | QMessageBox::No, QMessageBox::No) != QMessageBox::Yes)
        return;
    if (m_instance->isRunning() || !m_model->allMods().isEmpty() || QFileInfo::exists(m_model->dir().absoluteFilePath(selected.fileName)) ||
        !profile->getComponent("net.minecraft") || profile->getComponent("net.minecraft")->getVersion() != game || profile->getModLoaders() != loaders) {
        QMessageBox::warning(this, tr("Optimization"), tr("The instance changed during review. Nothing was installed; please check again."));
        return;
    }
    auto task = makeShared<ResourceDownloadTask>(pack, selected, m_model, true, "optimization");
    ProgressDialog download(this);
    download.setSkipButton(true, tr("Abort"));
    if (download.execWithTask(task.get()) != QDialog::Accepted || !task->wasSuccessful())
        QMessageBox::warning(this, tr("Optimization"), tr("The download failed or was cancelled. Check the mods folder before retrying."));
    m_model->update();
}

void ModFolderPage::downloadMods()
{
    if (m_instance->typeName() != "Minecraft") {
        return;  // this is a null instance or a legacy instance
    }

    auto* profile = static_cast<MinecraftInstance*>(m_instance)->getPackProfile();
    if (!profile->getModLoaders().has_value()) {
        if (handleNoModLoader()) {
            return;
        }
    }

    m_downloadDialog = new ResourceDownload::ModDownloadDialog(this, m_model, m_instance);
    connect(this, &QObject::destroyed, m_downloadDialog, &QDialog::close);
    connect(m_downloadDialog, &QDialog::finished, this, &ModFolderPage::downloadDialogFinished);

    m_downloadDialog->open();
}

void ModFolderPage::downloadDialogFinished(int result)
{
    if (result != 0) {
        auto* tasks = new ConcurrentTask(tr("Download Mods"), APPLICATION->settings()->get("NumberOfConcurrentDownloads").toInt());
        connect(tasks, &Task::failed, [this, tasks](const QString& reason) {
            CustomMessageBox::selectable(this, tr("Error"), reason, QMessageBox::Critical)->show();
            tasks->deleteLater();
        });
        connect(tasks, &Task::aborted, [this, tasks]() {
            CustomMessageBox::selectable(this, tr("Aborted"), tr("Download stopped by user."), QMessageBox::Information)->show();
            tasks->deleteLater();
        });
        connect(tasks, &Task::succeeded, [this, tasks]() {
            QStringList warnings = tasks->warnings();
            if (warnings.count()) {
                CustomMessageBox::selectable(this, tr("Warnings"), warnings.join('\n'), QMessageBox::Warning)->show();
            }

            tasks->deleteLater();
        });

        if (m_downloadDialog) {
            for (auto& task : m_downloadDialog->getTasks()) {
                tasks->addTask(task);
            }
        } else {
            qWarning() << "ResourceDownloadDialog vanished before we could collect tasks!";
        }

        ProgressDialog loadDialog(this);
        loadDialog.setSkipButton(true, tr("Abort"));
        loadDialog.execWithTask(tasks);

        m_model->update();
    }
    if (m_downloadDialog) {
        m_downloadDialog->deleteLater();
    }
}

void ModFolderPage::updateMods(bool includeDeps)
{
    if (m_instance->typeName() != "Minecraft") {
        return;  // this is a null instance or a legacy instance
    }

    auto* profile = static_cast<MinecraftInstance*>(m_instance)->getPackProfile();
    if (!profile->getModLoaders().has_value()) {
        if (handleNoModLoader()) {
            return;
        }
    }
    if (APPLICATION->settings()->get("ModMetadataDisabled").toBool()) {
        QMessageBox::critical(this, tr("Error"), tr("Mod updates are unavailable when metadata is disabled!"));
        return;
    }
    if (m_instance != nullptr && m_instance->isRunning()) {
        auto response =
            CustomMessageBox::selectable(this, tr("Confirm Update"),
                                         tr("Updating mods while the game is running may cause mod duplication and game crashes.\n"
                                            "The old files may not be deleted as they are in use.\n"
                                            "Are you sure you want to do this?"),
                                         QMessageBox::Warning, QMessageBox::Yes | QMessageBox::No, QMessageBox::No)
                ->exec();

        if (response != QMessageBox::Yes) {
            return;
        }
    }
    auto selection = m_filterModel->mapSelectionToSource(ui->treeView->selectionModel()->selection()).indexes();

    auto modsList = m_model->selectedResources(selection);
    bool useAll = modsList.empty();
    if (useAll) {
        modsList = m_model->allResources();
    }

    ResourceUpdateDialog updateDialog(this, m_instance, m_model, modsList, includeDeps, profile->getModLoadersList());
    updateDialog.checkCandidates();

    if (updateDialog.aborted()) {
        CustomMessageBox::selectable(this, tr("Aborted"), tr("The mod updater was aborted!"), QMessageBox::Warning)->show();
        return;
    }
    if (updateDialog.noUpdates()) {
        QString message{ tr("'%1' is up-to-date! :)").arg(modsList.front()->name()) };
        if (modsList.size() > 1) {
            if (useAll) {
                message = tr("All mods are up-to-date! :)");
            } else {
                message = tr("All selected mods are up-to-date! :)");
            }
        }
        CustomMessageBox::selectable(this, tr("Update checker"), message)->exec();
        return;
    }

    if (updateDialog.exec() != 0) {
        auto* tasks = new ConcurrentTask("Download Mods", APPLICATION->settings()->get("NumberOfConcurrentDownloads").toInt());
        connect(tasks, &Task::failed, [this, tasks](const QString& reason) {
            CustomMessageBox::selectable(this, tr("Error"), reason, QMessageBox::Critical)->show();
            tasks->deleteLater();
        });
        connect(tasks, &Task::aborted, [this, tasks]() {
            CustomMessageBox::selectable(this, tr("Aborted"), tr("Download stopped by user."), QMessageBox::Information)->show();
            tasks->deleteLater();
        });
        connect(tasks, &Task::succeeded, [this, tasks]() {
            QStringList warnings = tasks->warnings();
            if (warnings.count()) {
                CustomMessageBox::selectable(this, tr("Warnings"), warnings.join('\n'), QMessageBox::Warning)->show();
            }
            tasks->deleteLater();
        });

        for (const auto& task : updateDialog.getTasks()) {
            tasks->addTask(task);
        }

        ProgressDialog loadDialog(this);
        loadDialog.setSkipButton(true, tr("Abort"));
        loadDialog.execWithTask(tasks);

        m_model->update();
    }
}

void ModFolderPage::deleteModMetadata()
{
    auto selection = m_filterModel->mapSelectionToSource(ui->treeView->selectionModel()->selection()).indexes();
    auto selectionCount = m_model->selectedMods(selection).length();
    if (selectionCount == 0) {
        return;
    }
    if (selectionCount > 1) {
        auto response = CustomMessageBox::selectable(this, tr("Confirm Removal"),
                                                     tr("You are about to remove the metadata for %1 mods.\n"
                                                        "Are you sure?")
                                                         .arg(selectionCount),
                                                     QMessageBox::Warning, QMessageBox::Yes | QMessageBox::No, QMessageBox::No)
                            ->exec();

        if (response != QMessageBox::Yes) {
            return;
        }
    }

    m_model->deleteMetadata(selection);
}

void ModFolderPage::changeModVersion()
{
    if (m_instance->typeName() != "Minecraft") {
        return;  // this is a null instance or a legacy instance
    }

    auto* profile = static_cast<MinecraftInstance*>(m_instance)->getPackProfile();
    if (!profile->getModLoaders().has_value()) {
        if (handleNoModLoader()) {
            return;
        }
    }
    if (APPLICATION->settings()->get("ModMetadataDisabled").toBool()) {
        QMessageBox::critical(this, tr("Error"), tr("Mod updates are unavailable when metadata is disabled!"));
        return;
    }
    auto selection = m_filterModel->mapSelectionToSource(ui->treeView->selectionModel()->selection()).indexes();
    auto modsList = m_model->selectedMods(selection);
    if (modsList.length() != 1 || modsList[0]->metadata() == nullptr) {
        return;
    }

    m_downloadDialog = new ResourceDownload::ModDownloadDialog(this, m_model, m_instance, true);
    connect(this, &QObject::destroyed, m_downloadDialog, &QDialog::close);
    connect(m_downloadDialog, &QDialog::finished, this, &ModFolderPage::downloadDialogFinished);

    m_downloadDialog->setResourceMetadata((*modsList.begin())->metadata());
    m_downloadDialog->open();
}

void ModFolderPage::exportModMetadata()
{
    auto selection = m_filterModel->mapSelectionToSource(ui->treeView->selectionModel()->selection()).indexes();
    auto selectedMods = m_model->selectedMods(selection);
    if (selectedMods.length() == 0) {
        selectedMods = m_model->allMods();
    }

    std::ranges::sort(selectedMods, [](const Mod* a, const Mod* b) { return a->name() < b->name(); });
    ExportToModListDialog dlg(m_instance->name(), selectedMods, this);
    dlg.exec();
}

CoreModFolderPage::CoreModFolderPage(BaseInstance* inst, ModFolderModel* mods, QWidget* parent) : ModFolderPage(inst, mods, parent)
{
    auto* mcInst = dynamic_cast<MinecraftInstance*>(m_instance);
    if (mcInst) {
        auto* version = mcInst->getPackProfile();
        if ((version != nullptr) && version->getComponent("net.minecraftforge") && version->getComponent("net.minecraft")) {
            auto minecraftCmp = version->getComponent("net.minecraft");
            if (!minecraftCmp->m_loaded) {
                version->reload(Net::Mode::Offline);
                auto update = version->getCurrentTask();
                if (update) {
                    connect(update.get(), &Task::finished, this, [this] {
                        if (m_container) {
                            m_container->refreshContainer();
                        }
                    });
                    if (!update->isRunning()) {
                        update->start();
                    }
                }
            }
        }
    }
}

bool CoreModFolderPage::shouldDisplay() const
{
    if (ModFolderPage::shouldDisplay()) {
        auto* inst = dynamic_cast<MinecraftInstance*>(m_instance);
        if (!inst) {
            return true;
        }

        auto* version = inst->getPackProfile();
        if ((version == nullptr) || !version->getComponent("net.minecraftforge") || !version->getComponent("net.minecraft")) {
            return false;
        }
        auto minecraftCmp = version->getComponent("net.minecraft");
        return minecraftCmp->m_loaded && minecraftCmp->getReleaseDateTime() < g_VersionFilterData.legacyCutoffDate;
    }
    return false;
}

NilModFolderPage::NilModFolderPage(BaseInstance* inst, ModFolderModel* mods, QWidget* parent) : ModFolderPage(inst, mods, parent) {}

bool NilModFolderPage::shouldDisplay() const
{
    return m_model->dir().exists();
}

// Helper function so this doesn't need to be duplicated 3 times
inline bool ModFolderPage::handleNoModLoader()
{
    int resp = QMessageBox::question(
        this, ModFolderPage::tr("Missing Mod Loader"),
        ModFolderPage::tr("You need to install a compatible mod loader before installing mods. Would you like to do so?"),
        QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
    if (resp == QMessageBox::Yes) {
        // Should be safe
        auto* profile = static_cast<MinecraftInstance*>(this->m_instance)->getPackProfile();
        InstallLoaderDialog dialog(profile, QString(), this);
        bool ret = dialog.exec() != 0;
        this->m_container->refreshContainer();

        // returning negation of dialog.exec which'll be true if the install loader dialog got canceled/closed
        // and false if the user went through and installed a loader
        return !ret;
    }
    // Nothing happens the dialog is already closing
    // returning true so the caller doesn't go and continue with opening it's dialog without a mod loader
    return true;
}

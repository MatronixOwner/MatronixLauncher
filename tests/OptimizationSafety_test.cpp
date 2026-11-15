// SPDX-License-Identifier: GPL-3.0-only
#include <QTest>
#include "minecraft/mod/OptimizationSafety.h"

using namespace OptimizationSafety;
using namespace ModPlatform;

class OptimizationSafetyTest : public QObject {
    Q_OBJECT
  private slots:
    void checksDependenciesAndConflicts()
    {
        IndexedVersion version;
        version.addonId = "candidate";
        version.fileId = "candidate-version";
        InstalledOptimizationMod installed{ "library", "v1", true, {} };
        QVERIFY(candidateFits(version, {installed}));
        version.dependencies = { Dependency{ "library", DependencyType::REQUIRED, "v1" } };
        QVERIFY(candidateFits(version, {installed}));
        version.dependencies[0].version = "v2";
        QVERIFY(!candidateFits(version, {installed}));
        version.dependencies = { Dependency{ "library", DependencyType::INCOMPATIBLE, "" } };
        QVERIFY(!candidateFits(version, {installed}));
        installed.enabled = false;
        QVERIFY(candidateFits(version, {installed}));
        version.dependencies.clear();
        installed.projectId = "candidate";
        QVERIFY(!candidateFits(version, {installed}));
        installed.projectId = "library";
        installed.enabled = true;
        installed.dependencies = { Dependency{ "candidate", DependencyType::INCOMPATIBLE, "" } };
        QVERIFY(!candidateFits(version, {installed}));
        installed.dependencies = { Dependency{ "candidate", DependencyType::REQUIRED, "other-version" } };
        QVERIFY(!candidateFits(version, {installed}));
        installed.dependencies = { Dependency{ "candidate", DependencyType::UNKNOWN, "" } };
        QVERIFY(!candidateFits(version, {installed}));
    }
};
QTEST_GUILESS_MAIN(OptimizationSafetyTest)
#include "OptimizationSafety_test.moc"

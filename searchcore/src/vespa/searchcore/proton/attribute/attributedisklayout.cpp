// Copyright 2016 Yahoo Inc. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.

#include <vespa/fastos/fastos.h>
#include <vespa/log/log.h>
LOG_SETUP(".proton.attribute.attributedisklayout");
#include "attributedisklayout.h"
#include <vespa/searchcommon/common/schemaconfigurer.h>
#include <vespa/vespalib/io/fileutil.h>
#include "attribute_directory.h"

using search::IndexMetaInfo;
using search::index::SchemaBuilder;
using search::index::Schema;
using search::AttributeVector;

namespace proton
{

namespace {

vespalib::string
getSnapshotDir(uint64_t syncToken)
{
    return vespalib::make_string("snapshot-%" PRIu64, syncToken);
}

}

AttributeDiskLayout::AttributeDiskLayout(const vespalib::string &baseDir, PrivateConstructorTag)
    : _baseDir(baseDir),
      _mutex(),
      _dirs()
{
}

AttributeDiskLayout::~AttributeDiskLayout()
{
}

void
AttributeDiskLayout::createBaseDir()
{
    vespalib::mkdir(_baseDir, false);
}

vespalib::string
AttributeDiskLayout::getAttributeBaseDir(const vespalib::string &baseDir,
                                         const vespalib::string &attrName)
{
    if (baseDir.empty()) {
        return attrName;
    }
    return vespalib::make_string("%s/%s",
                                 baseDir.c_str(),
                                 attrName.c_str());
}

AttributeVector::BaseName
AttributeDiskLayout::getAttributeFileName(const vespalib::string &baseDir,
                                          const vespalib::string &attrName,
                                          uint64_t syncToken)
{
    return AttributeVector::BaseName(getAttributeBaseDir(baseDir, attrName),
                                     getSnapshotDir(syncToken),
                                     attrName);
}

std::vector<vespalib::string>
AttributeDiskLayout::listAttributes()
{
    std::vector<vespalib::string> attributes;
    std::shared_lock<std::shared_timed_mutex> guard(_mutex);
    for (const auto &dir : _dirs)  {
        attributes.emplace_back(dir.first);
    }
    return attributes;
}

void
AttributeDiskLayout::scanDir()
{
    FastOS_DirectoryScan dir(_baseDir.c_str());
    while (dir.ReadNext()) {
        if (strcmp(dir.GetName(), "..") != 0 && strcmp(dir.GetName(), ".") != 0) {
            if (dir.IsDirectory()) {
                createAttributeDir(dir.GetName());
            }
        }
    }
}

std::shared_ptr<AttributeDirectory>
AttributeDiskLayout::getAttributeDir(const vespalib::string &name)
{
    std::shared_lock<std::shared_timed_mutex> guard(_mutex);
    auto itr = _dirs.find(name);
    if (itr == _dirs.end()) {
        return std::shared_ptr<AttributeDirectory>();
    } else {
        return itr->second;
    }
}

std::shared_ptr<AttributeDirectory>
AttributeDiskLayout::createAttributeDir(const vespalib::string &name)
{
    std::unique_lock<std::shared_timed_mutex> guard(_mutex);
    auto itr = _dirs.find(name);
    if (itr == _dirs.end()) {
        auto dir = std::make_shared<AttributeDirectory>(shared_from_this(), name);
        auto insres = _dirs.insert(std::make_pair(name, dir));
        assert(insres.second);
        return dir;
    } else {
        return itr->second;
    }
}

void
AttributeDiskLayout::removeAttributeDir(const vespalib::string &name, search::SerialNum serialNum)
{
    auto dir = getAttributeDir(name);
    if (dir) {
        auto writer = dir->getWriter();
        if (writer) {
            writer->invalidateOldSnapshots(serialNum);
            if (writer->removeInvalidSnapshots(true)) {
                std::unique_lock<std::shared_timed_mutex> guard(_mutex);
                auto itr = _dirs.find(name);
                assert(itr != _dirs.end());
                assert(dir.get() == itr->second.get());
                _dirs.erase(itr);
                writer->detach();
            }
        } else {
            std::unique_lock<std::shared_timed_mutex> guard(_mutex);
            auto itr = _dirs.find(name);
            if (itr != _dirs.end()) {
                assert(dir.get() != itr->second.get());
            }
        }
    }
}

std::shared_ptr<AttributeDiskLayout>
AttributeDiskLayout::create(const vespalib::string &baseDir)
{
    auto diskLayout = std::make_shared<AttributeDiskLayout>(baseDir, PrivateConstructorTag());
    diskLayout->scanDir();
    return diskLayout;
}

std::shared_ptr<AttributeDiskLayout>
AttributeDiskLayout::createSimple(const vespalib::string &baseDir)
{
    auto diskLayout = std::make_shared<AttributeDiskLayout>(baseDir, PrivateConstructorTag());
    return diskLayout;
}

} // namespace proton


/*
 * Copyright (c) 2012, Willow Garage, Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the Willow Garage, Inc. nor the names of its
 *       contributors may be used to endorse or promote products derived from
 *       this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */
#ifndef BUILTIN_PLUGINLIB_FACTORY_H
#define BUILTIN_PLUGINLIB_FACTORY_H

#include <QHash>
#include <QString>
#include <QStringList>

#include <string>
#include <vector>

#include "rviz/class_id_recording_factory.h"
#include "rviz/load_resource.h"

namespace rviz
{
template <class Type>
class PluginlibFactory : public ClassIdRecordingFactory<Type>
{
private:
  struct BuiltInClassRecord
  {
    QString class_id_;
    QString package_;
    QString name_;
    QString description_;
    Type* (*factory_function_)();
  };

public:
  PluginlibFactory(const QString& package, const QString& base_class_type)
  {
  }
  ~PluginlibFactory() override
  {
  }

  QStringList getDeclaredClassIds() override
  {
    QStringList ids;
    typename QHash<QString, BuiltInClassRecord>::const_iterator iter;
    for (iter = built_ins_.begin(); iter != built_ins_.end(); iter++)
    {
      ids.push_back(iter.key());
    }
    return ids;
  }

  QString getClassDescription(const QString& class_id) const override
  {
    typename QHash<QString, BuiltInClassRecord>::const_iterator iter = built_ins_.find(class_id);
    if (iter == built_ins_.end())
    {
      return ""; // TODO: maybe raise exception? though maybe not a good idea in wasm...
    }
    return iter->description_;
  }

  QString getClassName(const QString& class_id) const override
  {
    typename QHash<QString, BuiltInClassRecord>::const_iterator iter = built_ins_.find(class_id);
    if (iter == built_ins_.end())
    {
      return ""; // TODO: maybe raise exception? though maybe not a good idea in wasm...
    }
    return iter->name_;
  }

  QString getClassPackage(const QString& class_id) const override
  {
    typename QHash<QString, BuiltInClassRecord>::const_iterator iter = built_ins_.find(class_id);
    if (iter == built_ins_.end())
    {
      return ""; // TODO: maybe raise exception? though maybe not a good idea in wasm...
    }
    return iter->package_;
  }

  virtual QString getPluginManifestPath(const QString& class_id) const
  {
    typename QHash<QString, BuiltInClassRecord>::const_iterator iter = built_ins_.find(class_id);
    if (iter == built_ins_.end())
    {
      return ""; // TODO: maybe raise exception? though maybe not a good idea in wasm...
    }
    return "";
  }

  QIcon getIcon(const QString& class_id) const override
  {
    QString package = getClassPackage(class_id);
    QString class_name = getClassName(class_id);
    QIcon icon = loadPixmap("package://" + package + "/icons/classes/" + class_name + ".svg");
    if (icon.isNull())
    {
      icon = loadPixmap("package://" + package + "/icons/classes/" + class_name + ".png");
      if (icon.isNull())
      {
        icon = loadPixmap("package://rviz/icons/default_class_icon.png");
      }
    }
    return icon;
  }

  virtual void addBuiltInClass(const QString& package,
                               const QString& name,
                               const QString& description,
                               Type* (*factory_function)())
  {
    BuiltInClassRecord record;
    record.class_id_ = package + "/" + name;
    record.package_ = package;
    record.name_ = name;
    record.description_ = description;
    record.factory_function_ = factory_function;
    built_ins_[record.class_id_] = record;
  }

protected:
  /** @brief Instantiate and return a instance of a subclass of Type using our
   *         pluginlib::ClassLoader.
   * @param class_id A string identifying the class uniquely among
   *        classes of its parent class.  rviz::GridDisplay might be
   *        rviz/Grid, for example.
   * @param error_return If non-NULL and there is an error, *error_return is set to a description of the
   * problem.
   * @return A new instance of the class identified by class_id, or NULL if there was an error.
   *
   * If makeRaw() returns NULL and error_return is not NULL,
   * *error_return will be set.  On success, *error_return will not be
   * changed. */
  Type* makeRaw(const QString& class_id, QString* error_return = nullptr) override
  {
    typename QHash<QString, BuiltInClassRecord>::const_iterator iter = built_ins_.find(class_id);
    if (iter == built_ins_.end())
    {
      return nullptr; // TODO: some kind of error
    }
    Type* instance = iter->factory_function_();
    if (instance == nullptr && error_return != nullptr)
    {
      *error_return = "Factory function for built-in class '" + class_id + "' returned NULL.";
    }
    return instance;
  }

private:
  QHash<QString, BuiltInClassRecord> built_ins_;
};

} // end namespace rviz

#endif // BUILTIN_PLUGINLIB_FACTORY_H

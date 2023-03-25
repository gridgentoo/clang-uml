/**
 * src/package_diagram/generators/json/package_diagram_generator.cc
 *
 * Copyright (c) 2021-2023 Bartek Kryza <bkryza@gmail.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "package_diagram_generator.h"

#include "util/error.h"

namespace clanguml::package_diagram::generators::json {

generator::generator(diagram_config &config, diagram_model &model)
    : common_generator<diagram_config, diagram_model>{config, model}
{
}

void generator::generate_relationships(
    const package &p, nlohmann::json &parent) const
{
    LOG_DBG("Generating relationships for package {}", p.full_name(true));

    // Generate this packages relationship
    if (m_model.should_include(relationship_t::kDependency)) {
        for (const auto &r : p.relationships()) {
            nlohmann::json rel = r;
            rel["source"] = std::to_string(p.id());
            json_["relationships"].push_back(std::move(rel));
        }
    }

    // Process it's subpackages relationships
    for (const auto &subpackage : p) {
        generate_relationships(
            dynamic_cast<const package &>(*subpackage), parent);
    }
}

void generator::generate(const package &p, nlohmann::json &parent) const
{
    LOG_DBG("Generating package {}", p.name());

    nlohmann::json j;
    j["id"] = std::to_string(p.id());
    j["name"] = p.name();
    j["type"] = "namespace";
    j["display_name"] = p.full_name(false);
    j["is_deprecated"] = p.is_deprecated();
    if (!p.file().empty())
        j["source_location"] =
            dynamic_cast<const common::model::source_location &>(p);
    if (const auto &comment = p.comment(); comment)
        j["comment"] = comment.value();

    for (const auto &subpackage : p) {
        auto &pkg = dynamic_cast<package &>(*subpackage);
        if (m_model.should_include(pkg)) {
            generate(pkg, j);
        }
    }

    parent["elements"].push_back(std::move(j));
}

void generator::generate(std::ostream &ostr) const
{
    if (m_config.using_namespace)
        json_["using_namespace"] = m_config.using_namespace().to_string();

    json_["name"] = m_model.name();
    json_["diagram_type"] = "package";

    json_["elements"] = std::vector<nlohmann::json>{};
    json_["relationships"] = std::vector<nlohmann::json>{};

    for (const auto &p : m_model) {
        auto &pkg = dynamic_cast<package &>(*p);
        if (m_model.should_include(pkg)) {
            generate(pkg, json_);
        }
    }

    // Process package relationships
    for (const auto &p : m_model) {
        if (m_model.should_include(dynamic_cast<package &>(*p)))
            generate_relationships(dynamic_cast<package &>(*p), json_);
    }

    ostr << json_;
}

} // namespace clanguml::package_diagram::generators::json
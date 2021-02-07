#!/usr/bin/env python

import json
import yaml
import urllib
import os
import sys

from jsonref import JsonRef  # type: ignore
import click


@click.command()
@click.option(
    "-o",
    "--output",
    default="schemas",
    metavar="PATH",
    help="Directory to store schema files",
)
@click.option(
    "-p",
    "--prefix",
    default="_definitions.json",
    help="Prefix for JSON references (only for OpenAPI versions before 3.0)",
)
@click.option(
    "--stand-alone", is_flag=True, help="Whether or not to de-reference JSON schemas"
)
@click.option(
    "--expanded", is_flag=True, help="Expand Kubernetes schemas by API version"
)
@click.option(
    "--kubernetes", is_flag=True, help="Enable Kubernetes specific processors"
)
@click.option(
    "--strict",
    is_flag=True,
    help="Prohibits properties not in the schema (additionalProperties: false)",
)
@click.argument("schema", metavar="SCHEMA_URL")
def default(output, schema, prefix, stand_alone, expanded, kubernetes, strict):
    """
    Converts a valid OpenAPI specification into a set of JSON Schema files
    """
    print("Downloading schema")
    if sys.version_info < (3, 0):
        response = urllib.urlopen(schema)
    else:
        if os.path.isfile(schema):
            schema = "file://" + os.path.realpath(schema)
        req = urllib.request.Request(schema)
        response = urllib.request.urlopen(req)
    print("Parsing schema")
    # Note that JSON is valid YAML, so we can use the YAML parser whether
    # the schema is stored in JSON or YAML
    data = yaml.load(response.read(), Loader=yaml.SafeLoader)
    if "swagger" in data:
        version = data["swagger"]
    elif "openapi" in data:
        version = data["openapi"]
    if not os.path.exists(output):
        os.makedirs(output)
    types = []
    print("Generating individual schemas")
    if version < "3":
        components = data["definitions"]
    else:
        components = data["components"]["schemas"]
    for title in components:
        kind = title.split(".")[-1]  # .lower()
        if kubernetes:
            group = title.split(".")[-3].lower()
            api_version = title.split(".")[-2].lower()
        specification = components[title]
        specification["$schema"] = "http://json-schema.org/schema#"
        specification.setdefault("type", "object")
        if strict:
            specification["additionalProperties"] = False
        if kubernetes and expanded:
            if group in ["core", "api"]:
                full_name = "%s-%s" % (kind, api_version)
            else:
                full_name = "%s-%s-%s" % (kind, group, api_version)
        else:
            full_name = kind
        types.append(title)
        try:
            print("Processing %s" % full_name)
            #print("change_dict_values is called, prefix is : %s" % prefix)
            # print("%s : %s" % (sys._getframe().f_lineno,
            #                   json.dumps(specification, indent=2)))
            updated = change_dict_values(specification, prefix, version)
            specification = updated
            if stand_alone:
                base = "file://%s/%s/" % (os.getcwd(), output)
                specification = JsonRef.replace_refs(
                    specification, base_uri=base)
            if "additionalProperties" in specification:
                if specification["additionalProperties"]:
                    updated = change_dict_values(
                        specification["additionalProperties"], prefix, version
                    )
                    specification["additionalProperties"] = updated
            if strict and "properties" in specification:
                updated = additional_properties(specification["properties"])
                specification["properties"] = updated
            if kubernetes and "properties" in specification:
                updated = replace_int_or_string(specification["properties"])
                updated = allow_null_optional_fields(updated)
                specification["properties"] = updated
            with open("%s/%s.json" % (output, full_name), "w") as schema_file:
                print("Generating %s.json" % full_name)
                schema_file.write(json.dumps(specification, indent=2))
        except Exception as e:
            print("An error occured processing %s: %s" % (kind, e))

    with open("%s/all.json" % output, "w") as all_file:
        print("Generating schema for all types")
        contents = {"oneOf": []}
        for title in types:
            print("Generating title %s" % title)
            titleList = title.split('/')
            refStr = "#/"+titleList[len(titleList)-1]+".json"
            contents["oneOf"].append(
                contents["oneOf"].append(
                    {"$ref": refStr}
                )
            )
        all_file.write(json.dumps(contents, indent=2))


def iteritems(d):
    if hasattr(dict, "iteritems"):
        return d.iteritems()
    else:
        return iter(d.items())


def additional_properties(data):
    "This recreates the behaviour of kubectl at https://github.com/kubernetes/kubernetes/blob/225b9119d6a8f03fcbe3cc3d590c261965d928d0/pkg/kubectl/validation/schema.go#L312"
    new = {}
    try:
        for k, v in iteritems(data):
            new_v = v
            if isinstance(v, dict):
                if "properties" in v:
                    if "additionalProperties" not in v:
                        v["additionalProperties"] = False
                new_v = additional_properties(v)
            else:
                new_v = v
            new[k] = new_v
        return new
    except AttributeError:
        return data


def replace_int_or_string(data):
    new = {}
    try:
        for k, v in iteritems(data):
            new_v = v
            if isinstance(v, dict):
                if "format" in v and v["format"] == "int-or-string":
                    new_v = {
                        "oneOf": [{"type": "string"}, {"type": "integer"}]}
                else:
                    new_v = replace_int_or_string(v)
            elif isinstance(v, list):
                new_v = list()
                for x in v:
                    new_v.append(replace_int_or_string(x))
            else:
                new_v = v
            new[k] = new_v
        return new
    except AttributeError:
        return data


def allow_null_optional_fields(data, parent=None, grand_parent=None, key=None):
    new = {}
    try:
        for k, v in iteritems(data):
            new_v = v
            if isinstance(v, dict):
                new_v = allow_null_optional_fields(v, data, parent, k)
            elif isinstance(v, list):
                new_v = list()
                for x in v:
                    new_v.append(allow_null_optional_fields(x, v, parent, k))
            elif isinstance(v, str):
                is_non_null_type = k == "type" and v != "null"
                has_required_fields = grand_parent and "required" in grand_parent
                is_required_field = (
                    has_required_fields and key in grand_parent["required"]
                )
                if is_non_null_type and not is_required_field:
                    new_v = [v, "null"]
            new[k] = new_v
        return new
    except AttributeError:
        return data


def change_dict_values(d, prefix, version):
    #print("change_dict_values is called, prefix is : %s" % prefix)
    new = {}
    try:
        for k, v in iteritems(d):
            new_v = v
            if isinstance(v, dict):
                new_v = change_dict_values(v, prefix, version)
            elif isinstance(v, list):
                new_v = list()
                for x in v:
                    new_v.append(change_dict_values(x, prefix, version))
            elif isinstance(v, str):
                if k == "$ref":
                    spList = v.split('/')
                    new_v = "#/"+spList[len(spList)-1]+".json"
            else:
                new_v = v
            new[k] = new_v
        return new
    except AttributeError:
        return d


def append_no_duplicates(obj, key, value):
    """
    Given a dictionary, lookup the given key, if it doesn't exist create a new array.
    Then check if the given value already exists in the array, if it doesn't add it.
    """
    if key not in obj:
        obj[key] = []
    if value not in obj[key]:
        obj[key].append(value)


if __name__ == "__main__":
    default()


# StripperCS2

A Metamod plugin that allows server operators to manage map lump data similarly to how [Stripper:Source](https://www.bailopan.net/stripper/) worked.

## Folder structure
Unlike Source, Source2 allows maps to include multiple lumps in the maps and reference each other. For example all entity templates will be turned into a new lump file. Stripper has to then also follow this.

The folder structure consists of 3 main parts, the map, the world name and the lump name. To avoid conflicts I have intentionally scoped all of the configs to maps, the world name and lump name come as a requirement from the engine.

The template of a stripper config file looks like `addons/StripperCS2/maps/<map>/<world name>/<lump name>.jsonc`

You can view all of the lumps a map has by using the `entity_lump_list` command

> [!CAUTION]
> `entity_lump_list` is a defensive command, it has to be either unlocked by a metamod plugin like CS2Fixes or defensive commands have to be disabled in the csgo_core/gameinfo.gi configuration

![image](https://github.com/Source2ZE/StripperCS2/assets/45881722/2ccacaf2-8a78-4ccb-99e5-14781465fd2e)

You can also see all the lumps in the map vpks under the `maps/<map>/entities` folder and read the raw keyvalues through [Source2Viewer](https://valveresourceformat.github.io/)

From the image above, if you wanted to edit entities inside the de_vertigo default_ents lump, you would have to create a `default_ents.jsonc` file under the `addons/StripperCS2/maps/de_vertigo` path. If you would want to edit the `default_ents` from `prefabs\misc\end_of_match` you would have to put the file inside `addons/StripperCS2/maps/de_vertigo/prefabs/misc/end_of_match`

## Configuration

StripperCS2 uses the JSON format instead of a custom config parser like Stripper:Source. Actions such as `filter`, `add`, `modify` are root arrays of the object.
You can use per map configs as explained in [Folder structure](#folder-structure) or you can use the global config in `addons/StripperCS2/global.jsonc` which will apply all the rules to every loaded lump.

### Filter
```json
{
    "filter": [
        {
          "classname": "/.*/"
        }
    ]
}
```

Similarly to Stripper:Source, each keyvalue is a string representation of its value, for example a color vector would be written as "255 0 0". Each key in the object is a keyvalue.
Every value can be either a string or a Perl regexp using the `/regex/` syntax.

#### Outputs

In StripperCS2 outputs are natively supported. An output is represented as an object under the reserved "io" field.

```json
{
    "filter": [
        {
          "io": [
            {
              "outputname": "OnMapStart"
            }
          ]
        }
    ]
}
```

This would filter all entities that have an OnMapStart output, the available keys are: `outputname`, `inputname`, `targetname`, `overrideparam`, `delay` (float), `timestofire` (int), `targettype` (int/EntityIOTargetType_t).
The enum for targettype can be acquired from the hl2sdk.
```c
enum EntityIOTargetType_t
{
	ENTITY_IO_TARGET_INVALID = -1,
	ENTITY_IO_TARGET_CLASSNAME = 0,
	ENTITY_IO_TARGET_CLASSNAME_DERIVES_FROM = 1,
	ENTITY_IO_TARGET_ENTITYNAME = 2,
	ENTITY_IO_TARGET_CONTAINS_COMPONENT = 3,
	ENTITY_IO_TARGET_SPECIAL_ACTIVATOR = 4,
	ENTITY_IO_TARGET_SPECIAL_CALLER = 5,
	ENTITY_IO_TARGET_EHANDLE = 6,
	ENTITY_IO_TARGET_ENTITYNAME_OR_CLASSNAME = 7,
}
```

### Add

```jsonc
{
    "add": [
        {
          "classname": "func_button",
          // ... other keyvalues
          "io": [
            {
              "outputname": "OnPressed",
              "targetname": "lockable_door",
              "inputname": "Lock"
            },
            // ... more io
          ]
        }
    ]
}
```

Adding new entities matches the exact system as `filter`, except instead of using the keyvalues for matching it adds them as a new entity, `io` array turns into a list of all the outputs that will be added. Regex values will be ignored.

### Modify

Modify is a more complex action that allows you to edit an already existing entity.
It has 4 sub objects matching Stripper:Source's behavior.

- `match`: All values in this objects will be used as a requirement for an entity to match, regular expressions are allowed for every value. `io` array can be used the same way as in `filter` action
- `replace`: Entity that passes the match requirements can have any of its keyvalues replaced with this sub object. Any keyvalue entered in here will be replaced with the right hand value. `io` **object** can also be used here, see [Replace Outputs](#replace-outputs) section
- `delete`: Deletes all they keyvalues that match their value with the right hand value in the json, this means you can match a single classname and then decide here whether a keyvalue will always be removed or only if it matches a value
- `insert`: Adds new keyvalues to the entity, works exactly the same as `add` action.

#### Replace Outputs

```jsonc
{
    "modify": [
        {
          "match":
          {
            // in this example this classname will match an entity with 3 outputs, OnFullyClosed, OnOpen and OnFullyOpen
            "classname": "func_door_rotating",
            // this array is not exhaustive, it will match any entity with at least these outputs
            // all these matched outputs will be then replaced with the replace object below
            "io": [
              {
                "outputname": "OnFullyClosed"
              },
              {
                "outputname": "OnOpen"
              }
            ]
          },
          "replace":
          {
            "io":
            // NOT ARRAY, ALL MATCHED IOs WILL BE REPLACED WITH THIS SINGLE OBJECT
            {
              // replaces only OnFullyClosed and OnOpen because they are in the match array, OnFullyOpen will be left alone
              "outputname": "CustomOutputName"
            },
          }
        }
    ]
}
```

Unlike Stripper:Source you don't need to delete and insert an output to modify it. You can use the `io` **OBJECT not array** to replace any value of the output. Note that this object is tightly connected with the match sub object. `replace` will only replace outputs that are explicitly matched inside the `io` array in the `match` sub object

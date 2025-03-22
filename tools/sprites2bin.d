/*
 * Convert a spritesheet json file to bin file
 * The json files to use with this tool can be generated with
 * https://github.com/odrick/free-tex-packer or https://www.leshylabs.com/apps/sstool/
 * export as JSON-TP-Hash then feed this program with it.
 */

import core.stdc.string;
import std.stdio;
import std.path;
import std.file;
import std.json;
import std.conv;

// We want to keep a 1 byte alignement
struct CatalogHeader
{
    align (1):
    char[6] id;
    int width, height, count;
    // char[2] padding is added by the compiler when the 1 byte alignement is not specified
}

struct CatalogEntry
{
    char[32] name;
    int width, height;
    float s, t, p, q;
}

void main(string[] args)
{
    // Check args
    if (args.length < 2) { writeln("Usage: sprites2bin file"); return; }
    if (!exists(args[1])) { writeln("File ", args[1], " does not exists."); return; }

    // Open file and generate json object
    auto inFilename = args[1];
    auto jsonString = readText(inFilename);
    auto json = parseJSON(jsonString, JSONOptions.none);
    writeln("Converting ", inFilename, "..."); 

    // Get image size and sprites objects
    auto imageSize = json["meta"].object["size"];
    auto imageSprites = json["frames"].object;

    // create output file
    auto outFilename = stripExtension(inFilename) ~ ".bin";
    auto outFile = File(outFilename, "w");
    scope (exit) { outFile.flush(); outFile.close(); }

    // Extract image size and sprite count
    auto imageWidth = to!int(imageSize["w"].integer);
    auto imageHeight = to!int(imageSize["h"].integer);
    auto spriteCount = to!int(imageSprites.length);

    // Write header
    auto header = CatalogHeader(
        "SPRCAT",
        imageWidth,
        imageHeight,
        spriteCount
    );

    outFile.rawWrite([header]);
    writeln(header);

    // Extract and write each sprites
    foreach (name, sprite; imageSprites)
    {
        char[32] entryName = 0;
        memcpy(entryName.ptr, name.ptr, name.length > 32 ? 32 : name.length);

        auto spriteX = to!int(sprite["frame"].object["x"].integer);
        auto spriteY = to!int(sprite["frame"].object["y"].integer);
        auto spriteWidth = to!int(sprite["frame"].object["w"].integer);
        auto spriteHeight = to!int(sprite["frame"].object["h"].integer);

        auto entry = CatalogEntry(
            entryName,
            spriteWidth,
            spriteHeight,
            to!float(spriteX) / to!float(imageWidth),
            to!float(spriteY) / to!float(imageHeight),
            (to!float(spriteX) + to!float(spriteWidth)) / to!float(imageWidth),
            (to!float(spriteY) + to!float(spriteHeight)) / to!float(imageHeight)
        );

        outFile.rawWrite([entry]);
        writeln(entry);
    }

}

using System;

namespace Unicodoid;

/// <summary>
/// SDK-wide entry points and version information.
/// </summary>
public static class UnicodoidSdk
{
    public const int VersionMajor = 1;
    public const int VersionMinor = 0;
    public const int VersionPatch = 0;
    public const string VersionString = "1.0.0";

    public static Version Version { get; } = new(VersionMajor, VersionMinor, VersionPatch);

    /// <summary>Initialize all SDK subsystems.  Idempotent.</summary>
    public static UnicodoidStatus Init() => EncodingSystemRegistry.Init();

    /// <summary>Shut down global SDK resources.</summary>
    public static void Shutdown() => EncodingSystemRegistry.Cleanup();
}

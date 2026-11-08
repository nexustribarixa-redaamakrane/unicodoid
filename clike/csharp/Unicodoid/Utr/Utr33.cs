namespace Unicodoid;

/// <summary>
/// UTR #33 Conformance Model (Revision 7).
///
/// Informative conformance model: levels of conformance and how the SDK maps
/// them to status codes.
/// </summary>
public static class Utr33
{
    /// <summary>Unspecified conformance level.</summary>
    public const int Unspecified = 0;

    /// <summary>Well-formed conformance level.</summary>
    public const int WellFormed = 1;

    /// <summary>Valid conformance level.</summary>
    public const int Valid = 2;

    /// <summary>Stable conformance level.</summary>
    public const int Stable = 3;

    /// <summary>Human-readable name for a conformance level.</summary>
    public static string LevelName(int level) => level switch
    {
        WellFormed => "Well_Formed",
        Valid => "Valid",
        Stable => "Stable",
        _ => "Unspecified"
    };
}

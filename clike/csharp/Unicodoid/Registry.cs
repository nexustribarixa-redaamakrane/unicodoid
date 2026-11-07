using System;
using System.Collections.Generic;
using Unicodoid.Systems;

namespace Unicodoid;

/// <summary>
/// Process-wide registry of encoding systems.  Registrations are borrowed
/// references; registered systems must remain alive for as long as they are
/// registered.  Built-in systems are process-wide singletons.
/// </summary>
public static class EncodingSystemRegistry
{
    private const int MaxRegisteredSystems = 64;

    private static readonly object s_gate = new();
    private static readonly List<IEncodingSystem> s_systems = new();
    private static readonly Dictionary<string, IEncodingSystem> s_byId =
        new(StringComparer.Ordinal);
    private static bool s_initialized;

    /// <summary>Register the built-in systems.  Idempotent.</summary>
    public static UnicodoidStatus Init()
    {
        lock (s_gate)
        {
            if (s_initialized) return UnicodoidStatus.Ok;

            s_systems.Clear();
            s_byId.Clear();

            RegisterCore(Utf8System.Instance);
            RegisterCore(Utf16LeSystem.Instance);
            RegisterCore(Utf16BeSystem.Instance);
            RegisterCore(Utf32LeSystem.Instance);
            RegisterCore(Utf32BeSystem.Instance);
            RegisterCore(AsciiSystem.Instance);
            RegisterCore(KupoConlangSystem.Instance);
            RegisterCore(ShiftModal16System.Instance);

            s_initialized = true;
            return UnicodoidStatus.Ok;
        }
    }

    /// <summary>Remove all registrations.</summary>
    public static void Cleanup()
    {
        lock (s_gate)
        {
            s_systems.Clear();
            s_byId.Clear();
            s_initialized = false;
        }
    }

    public static UnicodoidStatus Register(IEncodingSystem system)
    {
        if (system is null) return UnicodoidStatus.InvalidParam;
        if (string.IsNullOrEmpty(system.Id)) return UnicodoidStatus.InvalidParam;

        lock (s_gate)
        {
            if (s_byId.ContainsKey(system.Id)) return UnicodoidStatus.AlreadyExists;
            if (s_systems.Count >= MaxRegisteredSystems)
            {
                return UnicodoidStatus.CapacityExceeded;
            }

            s_systems.Add(system);
            s_byId.Add(system.Id, system);
            return UnicodoidStatus.Ok;
        }
    }

    public static UnicodoidStatus Unregister(string id)
    {
        if (id is null) return UnicodoidStatus.InvalidParam;

        lock (s_gate)
        {
            if (!s_byId.TryGetValue(id, out IEncodingSystem? system))
            {
                return UnicodoidStatus.NotFound;
            }

            s_byId.Remove(id);
            s_systems.Remove(system);
            return UnicodoidStatus.Ok;
        }
    }

    /// <summary>Find a system by ID; initializes the registry on first use.</summary>
    public static IEncodingSystem? FindById(string id)
    {
        if (id is null) return null;
        Init();

        lock (s_gate)
        {
            return s_byId.TryGetValue(id, out IEncodingSystem? system) ? system : null;
        }
    }

    /// <summary>Find a system by display name.</summary>
    public static IEncodingSystem? FindByName(string name)
    {
        if (name is null) return null;
        Init();

        lock (s_gate)
        {
            foreach (IEncodingSystem system in s_systems)
            {
                if (string.Equals(system.Name, name, StringComparison.Ordinal))
                {
                    return system;
                }
            }
            return null;
        }
    }

    public static int Count
    {
        get
        {
            Init();
            lock (s_gate)
            {
                return s_systems.Count;
            }
        }
    }

    /// <summary>Retrieve a system by index, or <c>null</c> when out of range.</summary>
    public static IEncodingSystem? GetAt(int index)
    {
        Init();
        lock (s_gate)
        {
            return index >= 0 && index < s_systems.Count ? s_systems[index] : null;
        }
    }

    /// <summary>Snapshot of all registered systems in registration order.</summary>
    public static IReadOnlyList<IEncodingSystem> All
    {
        get
        {
            Init();
            lock (s_gate)
            {
                return s_systems.ToArray();
            }
        }
    }

    private static void RegisterCore(IEncodingSystem system)
    {
        if (s_systems.Count >= MaxRegisteredSystems) return;
        if (s_byId.ContainsKey(system.Id)) return;

        s_systems.Add(system);
        s_byId.Add(system.Id, system);
    }
}

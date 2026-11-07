//! Process-wide registry of encoding systems.
//!
//! Registrations are borrowed references; registered systems must live for the
//! rest of the process.  Built-in systems are process-wide statics.

use std::sync::{Mutex, MutexGuard, OnceLock};

use crate::system::EncodingSystem;
use crate::systems::{ascii, custom_conlang, shiftmodal16, utf8, utf16, utf32};
use crate::types::Status;

const MAX_REGISTERED_SYSTEMS: usize = 64;

struct RegistryState {
    systems: Vec<&'static dyn EncodingSystem>,
    initialized: bool,
}

static REGISTRY: OnceLock<Mutex<RegistryState>> = OnceLock::new();

fn registry() -> &'static Mutex<RegistryState> {
    REGISTRY.get_or_init(|| {
        Mutex::new(RegistryState {
            systems: Vec::new(),
            initialized: false,
        })
    })
}

fn lock_registry() -> MutexGuard<'static, RegistryState> {
    registry()
        .lock()
        .unwrap_or_else(|poisoned| poisoned.into_inner())
}

fn builtins() -> [&'static dyn EncodingSystem; 8] {
    [
        utf8::system(),
        utf16::utf16le(),
        utf16::utf16be(),
        utf32::utf32le(),
        utf32::utf32be(),
        ascii::system(),
        custom_conlang::system(),
        shiftmodal16::system(),
    ]
}

fn register_locked(
    state: &mut RegistryState,
    system: &'static dyn EncodingSystem,
) -> Result<(), Status> {
    if system.id().is_empty() {
        return Err(Status::InvalidParam);
    }

    if state
        .systems
        .iter()
        .any(|existing| existing.id() == system.id())
    {
        return Err(Status::AlreadyExists);
    }

    if state.systems.len() >= MAX_REGISTERED_SYSTEMS {
        return Err(Status::CapacityExceeded);
    }

    state.systems.push(system);
    Ok(())
}

fn initialize(state: &mut RegistryState) -> Result<(), Status> {
    state.systems.clear();

    for system in builtins() {
        register_locked(state, system)?;
    }

    state.initialized = true;
    Ok(())
}

fn ensure_initialized(state: &mut RegistryState) -> Result<(), Status> {
    if !state.initialized {
        initialize(state)?;
    }
    Ok(())
}

/// Register the built-in systems.  Idempotent.
pub fn init() -> Result<(), Status> {
    let mut state = lock_registry();
    ensure_initialized(&mut state)
}

/// Remove every registration and return the registry to its uninitialized
/// state.  The next lookup or [`init`] call registers the built-ins again.
pub fn cleanup() {
    let mut state = lock_registry();
    state.systems.clear();
    state.initialized = false;
}

/// Register an additional system.
///
/// The registry is initialized first so that a registration performed before
/// the first lookup is not discarded.
pub fn register(system: &'static dyn EncodingSystem) -> Result<(), Status> {
    let mut state = lock_registry();
    ensure_initialized(&mut state)?;
    register_locked(&mut state, system)
}

/// Remove a system by identifier.
pub fn unregister(id: &str) -> Result<(), Status> {
    let mut state = lock_registry();
    ensure_initialized(&mut state)?;

    if let Some(index) = state.systems.iter().position(|system| system.id() == id) {
        state.systems.remove(index);
        Ok(())
    } else {
        Err(Status::NotFound)
    }
}

/// Find a system by identifier.
pub fn find_by_id(id: &str) -> Option<&'static dyn EncodingSystem> {
    let mut state = lock_registry();
    ensure_initialized(&mut state).ok()?;
    state
        .systems
        .iter()
        .find(|system| system.id() == id)
        .copied()
}

/// Find a system by display name.
pub fn find_by_name(name: &str) -> Option<&'static dyn EncodingSystem> {
    let mut state = lock_registry();
    ensure_initialized(&mut state).ok()?;
    state
        .systems
        .iter()
        .find(|system| system.name() == name)
        .copied()
}

/// Number of registered systems.
pub fn count() -> usize {
    let mut state = lock_registry();
    let _ = ensure_initialized(&mut state);
    state.systems.len()
}

/// Retrieve a registered system by position.
pub fn get_at(index: usize) -> Option<&'static dyn EncodingSystem> {
    let mut state = lock_registry();
    ensure_initialized(&mut state).ok()?;
    state.systems.get(index).copied()
}

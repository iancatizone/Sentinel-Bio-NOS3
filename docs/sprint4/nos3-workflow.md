# NOS3 File Architecture and Working Procedure

Status: explanation and workflow outline. Mission-specific install/build/run commands are intentionally absent until the source and integration approach are tested.

## Do I replace files inside the stock NOS3 folder?

Do not manually overwrite the stock baseline. Keep `SBP-NOS3-BL-01` recoverable, preserve source revisions, and make any mission integration changes on a dedicated branch/fork or through a controlled project overlay.

The current repository holds baseline documentation and now planning outlines, not a vendored full NOS3 distribution. The FlatSat software, NOS3 dependency, and project integration must each have a clear source of truth.

The pinned NOS3 tree includes `cfg/` for configuration sources, `components/` for component code, `fsw/` for flight-software frameworks, and simulator build targets under `sims/` ([Makefile](https://raw.githubusercontent.com/nasa/nos3/5a3bdee6be9a2c67fdf994ae6db56d5c60395302/Makefile), [existing component integration landmarks](../sprint3/architecture-note.md)).

## Can I make a mission folder?

Yes, as a project organization convention. The [mission manifest outline](../../missions/sentinel-bio-flatsat/README.md) reserves a home for source/configuration pins; it is not yet a runnable mission.

Creating that folder does not install anything into NOS3. The pinned configuration script starts from `cfg/nos3-mission.xml`, copies source files into `cfg/build/`, and supports a spacecraft-config override under `cfg/` ([configuration script](https://raw.githubusercontent.com/nasa/nos3/5a3bdee6be9a2c67fdf994ae6db56d5c60395302/scripts/cfg/config.sh)).

Select either:

- **Tracked NOS3 integration branch/fork:** Review changes directly against the pinned stock revision and keep model/app dependencies explicitly versioned.
- **Project overlay:** Keep mission/model source here and provide a tested staging/build process into the dependency checkout, with a manifest and clear ownership of staged files.

The choice is open. Do not use undocumented copying as the long-term workflow or assume an arbitrary `missions/` directory is discovered by stock NOS3.

## Do I redo `vagrant up`?

No new VM is normally required for a new component or mission configuration. At the pinned baseline, Vagrant shares the host NOS3 checkout into `/home/jstar/Desktop/github-nos3`; NOS3 configures/builds/launches the software separately ([Vagrantfile](https://raw.githubusercontent.com/nasa/nos3/5a3bdee6be9a2c67fdf994ae6db56d5c60395302/Vagrantfile), [Makefile](https://raw.githubusercontent.com/nasa/nos3/5a3bdee6be9a2c67fdf994ae6db56d5c60395302/Makefile)).

`vagrant up` can start the existing VM. It is not an instruction to reinstall the mission, destroy the VM, or reprovision it after every code change.

A future project checkout beside the NOS3 checkout inside the VM is possible without replacing the stock tree, but it is not automatically a host-shared directory. Document storage/sharing and persistence before finalizing setup instructions.

## Planned daily cycle

- **Stop:** Stop only the current simulation/project services before changing their build/configuration.
- **Update:** Pull tracked project and approved dependency revisions; inspect local changes first.
- **Configure:** Regenerate when source configuration changes; do not edit generated `cfg/build` files as masters.
- **Build:** Rebuild affected model/adapter/app targets and dependencies.
- **Launch:** Use the tested mission procedure once implemented.
- **Test:** Run relevant checks; capture formal evidence for an acceptance/regression claim, not every boot.

Do not introduce blanket cleanup, root permissions, raw host-device access, or VM recreation just to make simulation tests work.

## Required before releasing executable instructions

- [ ] Actual Python/Docker source and RockBLOCK caller recovered.
- [ ] Versioned integration method and source pins chosen.
- [ ] App transport redirection and clock behavior tested.
- [ ] Component build and startup registration documented.
- [ ] Repeatable simulation-only launch works without hardware device mounts.
- [ ] Stock baseline remains separately reproducible.

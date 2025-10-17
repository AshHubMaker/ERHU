# Erhu Tuner (Flipper Zero) — Click-to-Build Repo

Use this repo to compile the app via **GitHub Actions** (no Terminal).

## Steps (no Terminal)
1. Create a new GitHub repo on your account.
2. Upload everything in this folder (keep `.github/workflows/build.yml`).
3. Open the **Actions** tab → enable workflows if asked.
4. Choose **Build Flipper App** → **Run workflow**.
5. When it finishes, open the run → **Artifacts** → download `erhu_tuner_build`.
6. Inside the artifact, find the `.fap` and copy it to your Flipper SD: `apps/Media/`.

## Controls
- LEFT = Inner (D4)
- RIGHT = Outer (A4)
- OK = Toggle play/stop
- BACK (hold) = Quit
- UP/DOWN = Volume +/-

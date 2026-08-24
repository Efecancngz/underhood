# Handoff — underhood
Son güncelleme: 2026-08-24 ~16:45, güncelleyen: Claude Sonnet 5

## Şu an ne yapılıyor
Subagent-driven-development ile plan uygulanıyor (docs/superpowers/plans/2026-08-24-underhood-core-and-first-modules.md), izole worktree'de (`.worktrees/core-and-first-modules`, branch `feat/core-and-first-modules`). Task 1-8 tamamlandı ve review'dan geçti. Task 9 (shared_ptr modülü, commit `109da37`) kod olarak yazıldı ve commit edildi ama build/test ile bağımsız doğrulanamadı — oturum zaman kısıtı nedeniyle duraklatıldı (kullanıcı bilgisayarı kapatacaktı).

## Sıradaki somut adım
1. `cd .worktrees/core-and-first-modules`, `rm -rf build` (önceki build kirli/kısmi durumda kaldı, gitignored, güvenle silinebilir).
2. `GIT_SHALLOW TRUE`'nun neden raylib'i tekrar büyüttüğünü araştır (bkz. SDD ledger'daki not — `.superpowers/sdd/2026-08-24-underhood-core-and-first-modules/progress.md`) — muhtemelen `GIT_TAG 5.5` annotated tag olduğu için ek bir fetch tetikleniyor olabilir, `GIT_TAG` yerine commit hash denenebilir.
3. Temiz `cmake -S . -B build && cmake --build build --parallel && ctest --test-dir build --output-on-failure` çalıştır, gerçekten geçtiğini doğrula.
4. Task 9 için review-package script'ini çalıştır (BASE=f61cddf, HEAD=109da37 — sadece modül diff'i, sonraki iki housekeeping commit'i (4a46517, 289e91f) hariç) ve reviewer subagent'ı dispatch et.
5. Onaylanırsa Task 10 (move_semantics) ile devam et, plan dosyasındaki sıradaki görevler aynen izlenir.

## Bilinmesi gerekenler
- **OBJECT library kararı bilinçli:** STATIC library kullanılırsa self-registration global'leri linker tarafından silinebilir (Task 8'de doğrulandı, deseni Task 9/10 kopyalıyor).
- **Meyers' singleton registry:** static initialization order fiasco riskini önlemek için bilinçli (Task 4'te doğrulandı).
- **Denenip başarısız olan:** Task 4/5 subagent'ları arka plan build komutlarını başlatıp sonucunu kontrol etmeden turlarını bitirdi — bu, worktree'de birikmiş kilitli/orphan cmake-git-MSBuild süreçlerine yol açtı (Task 9 review öncesi keşfedildi ve temizlendi). Gelecekte bir subagent "arka planda bekliyorum" deyip turunu bitirirse hemen SendMessage ile "senkron çalıştır, sonucu kontrol et" diye müdahale et.
- **raylib GIT_SHALLOW TRUE tam çalışmıyor gibi görünüyor** — ilk denemede 15-30M'ye indi ama sonra 55M'ye tekrar büyüdü, kök neden araştırılmadı, yukarıdaki "sıradaki adım"a bak.
- `mcp.json` (GitHub PAT, push erişimi için) hem `master`'da hem bu branch'te `.gitignore`'a eklendi (commit `87e878b` master'da, `4a46517` bu branch'te) — asla commit edilmemeli.

## İlgili dosyalar
- docs/superpowers/specs/2026-08-24-underhood-core-design.md — tam tasarım kararları
- docs/superpowers/plans/2026-08-24-underhood-core-and-first-modules.md — implementasyon planı (Task 5/6/8/9/10/11'de mid-flight düzeltmeler var, plan güncel)
- .superpowers/sdd/2026-08-24-underhood-core-and-first-modules/progress.md — SDD ledger, tüm rulings ve task geçmişi burada

## Son 3 commit
- 289e91f chore: use shallow git clones for FetchContent dependencies
- 4a46517 chore: ignore mcp.json (local credentials)
- 109da37 feat: add shared_ptr simulation module

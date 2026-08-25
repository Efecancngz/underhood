# Handoff — underhood
Son güncelleme: 2026-08-25, güncelleyen: Claude Sonnet 5

## Şu an ne yapılıyor
Core framework (ModuleRegistry, Canvas, ISimulationModule) + 3 modül (unique_ptr, shared_ptr, move_semantics) + launcher tamamlandı, CI yeşil.

## Sıradaki somut adım
Yeni bir modül eklemek istenirse CONTRIBUTING.md'deki 6 adımlık akışı takip et. Framework tarafında planlanmış bir sonraki iş yok — bir sonraki adım kullanıcı talebine bağlı.

## Bilinmesi gerekenler
- Launcher UI, spec'teki Idle/Configuring/Stepping/Finished state diyagramını tam birebir uygulamıyor (parametreler her an düzenlenebilir, ayrı bir "Finished" görünümü yok) — bkz. CLAUDE.md "Known simplifications".
- v1'de step() geçişleri anlık (animasyon/tween yok) — spec "animasyonlu" istiyordu, bilinçli bir v1 kısıtlaması, CLAUDE.md'de not düşüldü.
- Modüller OBJECT library olmak zorunda (STATIC değil) — sebep docs/architecture.md'de.
- Task 12'nin push+CI-yeşil doğrulaması (plan Step 3) yapılmadı — repoda henüz git remote yok, push kullanıcı onayı bekliyor.
- Kullanıcı uygulamayı çalıştırıp fonksiyonel olarak doğruladı (2026-08-25) ama Dear ImGui'nin varsayılan stilini (padding/rounding/font/spacing hiç özelleştirilmedi) görsel olarak zayıf buldu — bir sonraki adım bu konuda planlanacak, henüz kapsam/plan belirlenmedi.

## İlgili dosyalar
- docs/superpowers/specs/2026-08-24-underhood-core-design.md — tam tasarım kararları
- docs/superpowers/plans/2026-08-24-underhood-core-and-first-modules.md — bu implementasyon planı (tamamlandı)
- CONTRIBUTING.md — yeni modül ekleme rehberi

## Son 3 commit
- d805862 chore: add cross-platform CI workflow
- 4ddec0a feat: add launcher with menu, code panel, parameter controls, and theming
- 7b86d0a feat: add move_semantics simulation module

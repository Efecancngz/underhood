# Handoff — underhood
Son güncelleme: 2026-08-25, güncelleyen: Claude Sonnet 5

## Şu an ne yapılıyor
Core framework + 3 "Smart Pointers" modülü (unique_ptr, shared_ptr, move_semantics) + 4 "Data Structures" modülü (stack, queue, linked_list_singly, linked_list_circular) + launcher tamamlandı. İki modül türü var: IStepSimulationModule (sabit senaryo) ve IOperationalModule (açık uçlu Push/Pop tarzı operasyonlar + animasyon + geçmiş). CI yeşil (Task 12'nin push+doğrulama adımı hâlâ bekliyor, ayrı not).

## Sıradaki somut adım
Yeni bir modül eklemek istenirse CONTRIBUTING.md'deki akışı takip et (önce IStepSimulationModule mi IOperationalModule mü karar ver). Framework tarafında planlanmış bir sonraki iş yok.

## Bilinmesi gerekenler
- Circular linked list'in "başa dönüş" oku gerçek bir kavis değil, düz çizgi + etiket (bkz. docs/superpowers/specs/2026-08-25-underhood-operational-modules-design.md "Bilinen sadeleştirmeler").
- Tüm Data Structures modülleri max 8 eleman, kullanıcı tarafından ayarlanamaz.
- Task 12'nin push+CI-yeşil doğrulaması hâlâ yapılmadı — repoda henüz git remote yok.

## İlgili dosyalar
- docs/superpowers/specs/2026-08-24-underhood-core-design.md — çekirdek framework tasarımı
- docs/superpowers/specs/2026-08-25-underhood-operational-modules-design.md — bu planın tasarımı
- docs/superpowers/plans/2026-08-25-underhood-operational-modules.md — bu implementasyon planı (tamamlandı)
- CONTRIBUTING.md — yeni modül ekleme rehberi (her iki modül türü için)

## Son 3 commit
- 0d1ab5f feat: add circular linked list simulation module
- 0cc13fa feat: add singly linked list simulation module
- 5f0717c feat: add queue simulation module

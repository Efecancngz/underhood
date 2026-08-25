# underhood — Açık Uçlu Operasyon Modülleri (Data Structures) — Tasarım

**Tarih:** 2026-08-25
**Durum:** Onaylandı, implementasyon planı bekleniyor

## 0. Planlama & Analiz

### Problem tanımı
Mevcut `ISimulationModule` sözleşmesi tek bir senaryo türüne kilitli: `reset()` başlangıç durumunu kurar, `step()` her tıklamada önceden kurgulanmış sabit bir senaryonun bir sonraki adımına geçer (örn. "paylaş, sonra a'yı sıfırla, sonra b'yi sıfırla"). Veri yapıları (stack, queue, linked list) için bu model referans alınan araçlardaki (visualgo.net, dsa-visualizer) deneyimle örtüşmüyor: o araçlarda kullanıcı istediği değeri istediği anda push/pop/insert/delete edebiliyor, kod bloğundan çok "işleyiş" (canlı durum + geçmiş) öne çıkıyor.

### Gereksinimler

**Fonksiyonel:**
- Kullanıcı bir veri yapısı seçtiğinde, değer girip Push/Pop/Enqueue/Dequeue/Insert/Delete gibi butonlarla o an istediği operasyonu tetikleyebilir (sabit bir senaryo yok).
- Her operasyon kısa bir animasyonla oynanır (~300ms, ease-out) — kutu ekleniyorsa kayarak/büyüyerek belirir, siliniyorsa soluklaşarak/kayarak çıkar.
- Yapılan işlemlerin kısa bir geçmişi (Operation History) görünür kalır.
- Mevcut 3 modül (`unique_ptr`, `shared_ptr`, `move_semantics`) davranış olarak hiç değişmez — aynı kod, aynı testler, aynı sonuç.
- 4 yeni modül: Stack, Queue, Singly Linked List, Circular Linked List — yeni "Data Structures" kategorisi altında.
- Kod paneli bu 4 modülde boş kalır (`codeSnippet()` boş string döner) — işleyiş öne çıkar, kod zorunlu değil.

**Fonksiyonel olmayan:**
- Mevcut modüller ve testleri hiç dokunulmadan geçmeli (`ctest` aynı 7 test + yeni testler, hepsi yeşil).
- Yeni bir modül türü eklemek (üçüncü bir "kind") gelecekte launcher'ı yeniden yazmayı gerektirmemeli — launcher `kind()`'a göre dallanır, yeni kind eklemek yeni bir case eklemek kadar basit kalmalı.
- 4 modülün hepsi aynı animasyon/geçmiş altyapısını paylaşır (kod tekrarı yerine ortak `core` yardımcıları).

**İş gereksinimleri:**
- Paydaş: Efecan. Başarı kriteri: 4 modül çalışıyor, animasyonlu, geçmiş paneli var, mevcut 3 modül bozulmadı, CI yeşil.

### Sıfırdan mı, açık kaynaktan mı (standart §0.4)
Framework zaten sıfırdan yazılıyor (bkz. 2026-08-24 core design'ın §7'si) — bu doküman onun üzerine bir uzantı, aynı gerekçe geçerli: hazır bir "modüler + katkıya açık + adım-adım" C++ masaüstü aracı yok.

### API kontratı — arayüz ayrımı

`ISimulationModule` tek başına hem step-senaryo hem açık-uçlu-operasyon modüllerini karşılayamıyor (biri `step()`/`currentHighlightedLine()` ister, diğeri `performOperation()`/`history()` — ortak zemin çok az). Tek "her şeyi yapan" arayüz yerine üç seviyeli bir ayrım:

```cpp
enum class ModuleKind { Step, Operational };

class ISimulationModule {
public:
    virtual ~ISimulationModule() = default;
    virtual std::string name() const = 0;
    virtual std::string codeSnippet() const = 0;   // Operational modüllerde "" dönebilir
    virtual void render(Canvas& canvas) const = 0;
    virtual ModuleKind kind() const = 0;
};

class IStepSimulationModule : public ISimulationModule {
public:
    ModuleKind kind() const override { return ModuleKind::Step; }
    virtual std::vector<Parameter> parameters() const = 0;
    virtual void reset(const std::vector<Parameter>& params) = 0;
    virtual bool step() = 0;
    virtual int currentHighlightedLine() const = 0;
};

class IOperationalModule : public ISimulationModule {
public:
    ModuleKind kind() const override { return ModuleKind::Operational; }
    struct Operation {
        std::string label;      // buton metni, örn. "Push"
        bool takesValue;        // true ise launcher değer kutusunu gösterir
    };
    virtual std::vector<Operation> operations() const = 0;
    virtual bool canPerform(const std::string& operationLabel) const = 0;  // buton enable/disable
    virtual void performOperation(const std::string& operationLabel, int value) = 0;
    virtual std::vector<std::string> history() const = 0;  // en yeni sonda; launcher son N'i gösterir
    virtual void update(float deltaTime) = 0;               // animasyonları ilerletir, her frame çağrılır
};
```

`codeSnippet()` tabana taşındı (önceden `IStepSimulationModule`'a özeldi) çünkü launcher'ın Code panelini "boşsa gizle" mantığı her iki türde de aynı kalsın istiyoruz; `IOperationalModule` somut sınıfları bu metottan `""` döner.

`ModuleRegistry::create()` değişmiyor — yine `std::unique_ptr<ISimulationModule>` döner. Launcher, `module->kind()`'a bakıp `static_cast<IStepSimulationModule*>` ya da `static_cast<IOperationalModule*>` ile ilgili panel setini çizer (kind zaten doğru tipi garanti ettiği için `dynamic_cast`/RTTI'ye gerek yok).

Mevcut 3 modül: tek satırlık değişiklik — `class UniquePtrModule : public underhood::ISimulationModule` yerine `: public underhood::IStepSimulationModule`. Metot gövdeleri, testler, davranış — hiçbiri değişmiyor.

### Ortak animasyon altyapısı (core)

Her 4 modülün ayrı ayrı yeniden yazmaması için `core`'a iki küçük, bağımsız yardımcı:

```cpp
// core/include/underhood/animation.hpp
class Animator {
public:
    void start();                 // t=0'dan başlat
    void update(float deltaTime); // ilerlet
    float progress() const;       // 0..1, ease-out uygulanmış (kararlı: bitince hep 1 döner)
    bool isAnimating() const;
private:
    float elapsed_ = 0.0f;
    bool active_ = false;
    static constexpr float kDurationSeconds = 0.30f;
};

// core/include/underhood/animated_list.hpp
template <typename T>
class AnimatedList {
public:
    struct Entry { T value; Animator insertAnim; };

    void insertAt(std::size_t index, T value);  // entries_'e ekler, insertAnim.start()
    void removeAt(std::size_t index);           // entries_'ten çıkarır, removingEntry_'e taşır, onun animator'ını start eder
    void update(float deltaTime);                // tüm entries_ + removingEntry_ (varsa) ilerletilir; removingEntry_ biterse temizlenir
    const std::vector<Entry>& entries() const;
    const Entry* removingEntry() const;           // nullptr = şu an silinen yok
    std::size_t size() const;
    bool empty() const;
    bool full(std::size_t maxSize) const;

private:
    std::vector<Entry> entries_;
    std::optional<Entry> removingEntry_;
};
```

`removeAt` çağrıldığında zaten bir `removingEntry_` varsa (önceki silme animasyonu henüz bitmeden yeni bir silme tetiklendiyse — pratikte nadir ama olası), önceki `removingEntry_` animasyonsuz düşürülür ve yenisiyle değiştirilir; iki eş zamanlı fade-out kuyruğa alınmaz. `history()` en yeni kayıt sonda olacak şekilde döner (append-only log); launcher görüntülerken listeyi tersten okuyup en yeni kaydı üstte gösterir — `history()`'nin kendisi ters çevrilmez.

`AnimatedList<int>` dört modülün de state'inin omurgası: Stack `push` = `insertAt(size())`, `pop` = `removeAt(size()-1)`; Queue `enqueue` = `insertAt(size())`, `dequeue` = `removeAt(0)`; her iki linked list `insertFront` = `insertAt(0)`, `insertBack`/`append` = `insertAt(size())`, `deleteFront` = `removeAt(0)`. Modüller `render()`'da `entries()` ve `removingEntry()` üzerinden `progress()`'e göre kutu pozisyonu/opaklığını hesaplayıp `canvas.drawBox`/`drawArrow` çağırır — `Canvas`'a hiçbir yeni metot eklenmiyor.

`update(float deltaTime)` çağrısı `main()`'in ana döngüsünden, aktif modül `Operational` ise her frame `activeModule->update(GetFrameTime())` şeklinde yapılır (kullanıcı tıklamasa bile animasyon aksın diye).

### Kapasite ve sınır durumları
- 4 yapı da **max 8 eleman**. Dolduğunda ekleyen operasyonlar `canPerform()` ile `false` döner, launcher butonu devre dışı bırakır (disabled, tıklanamaz — sessiz no-op yerine açık bir sinyal).
- Boş yapıda çıkaran operasyonlar (Pop/Dequeue/Delete Front) aynı şekilde `canPerform() == false` ile devre dışı.
- Reset/yeniden başlatma: `IOperationalModule`'da `reset()` yok (senaryo kavramı yok) — her modülün kendi "Clear" operasyonu (listede yer almıyor, launcher'da ayrı bir "Clear" butonu, her operasyonel modülde ortak) tüm `entries_`'i anlık temizler (animasyonsuz — geçmişi sıfırlamak "geri al" değil "en baştan başla" anlamına gelir).

### 4 modülün somut operasyonları

| Modül | Operasyonlar | Görsel yerleşim |
|---|---|---|
| **Stack** | Push(value), Pop | Dikey kutu yığını (alttan yukarı), en üstteki (son push edilen) vurgulu kenarlıkla işaretli |
| **Queue** | Enqueue(value), Dequeue | Yatay kutu sırası, sol uç "front", sağ uç "back" etiketli |
| **Singly Linked List** | Insert Front(value), Insert Back(value), Delete Front | Yatay zincir, `drawArrow` ile ardışık bağlantı, sonda "null" kutusu |
| **Circular Linked List** | Insert(value) [her zaman sona ekler], Delete Front | Yatay zincir + son elemandan ilk elemana dönen eğri bir `drawArrow` çağrısı (mevcut düz ok API'si iki nokta arası düz çizgi çiziyor; dönüş oku için elemanların üstünden geçen bir kavis yerine, ilk sürümde son kutudan ilk kutuya doğrudan çapraz bir çizgi + "wraps to front" etiketi — gerçek eğri çizim v2'ye bırakılıyor, bkz. Bilinen sadeleştirmeler) |

### Launcher değişiklikleri
- **Controls paneli:** `kind()==Step` ise mevcut hâliyle kalır (parametre kutuları + Reset + Next Step). `kind()==Operational` ise: değer girişi (`InputInt`) + her `Operation` için bir buton (`canPerform()` false ise `ImGui::BeginDisabled()`) + "Clear" butonu + altında sabit yükseklikte kaydırılabilir bir `ImGui::BeginChild` içinde `history()`'nin son ~10 satırı (en yeni üstte).
- **Code paneli:** `codeSnippet()` boşsa "Select a simulation..." yerine hiçbir şey göstermez (boş panel) — modül seçili değilken hâlâ placeholder mesajı kalır, sadece "seçili ama kod yok" durumu farklı.
- **Modules sidebar:** `ModuleRegistry::categories()` zaten kategorileri destekliyor (2026-08-25'te eklendi) — yeni modüller `"Data Structures"` kategorisiyle kayıt olur, launcher'da hiçbir değişiklik gerekmiyor.
- **Ana döngü:** `activeModule` `Operational` ise her frame `update(GetFrameTime())` çağrılır (bir `dynamic_cast`/`kind()` kontrolüyle).

### Test yaklaşımı
Her modülün state mantığı (AnimatedList kullanımı, canPerform sınırları, history string'leri) GUI'siz `Catch2` testleriyle doğrulanır — mevcut 3 modülün test paternini birebir izler (`tests/test_stack_module.cpp` vb.). `Animator`/`AnimatedList` da kendi başına test edilir (`tests/test_animation.cpp`, `tests/test_animated_list.cpp`): `update()` ile `progress()`'in 0→1 ilerlediği, `isAnimating()`'in doğru düştüğü, `removeAt` sonrası `removingEntry()`'nin animasyon bitince `nullptr`'a döndüğü.

### Bilinen sadeleştirmeler (v1)
- Circular linked list'in "başa dönüş" oku gerçek bir kavis değil, düz çapraz çizgi + etiket — gerçek eğri çizim (bezier/arc) `Canvas::drawArrow`'a yeni bir API gerektirir, bu iterasyonun kapsamı dışında.
- Kapasite sabit 8, kullanıcı tarafından ayarlanamaz.
- "Clear" animasyonsuz — anlık temizlik.
- Operation history sabit son ~10 kayıt, daha eskisi tutulmuz (sınırsız geçmiş/undo yok).

---

## Onay
Bölüm bölüm kullanıcı onayından geçti (2026-08-25): arayüz ayrımı, animasyon altyapısı, operation history, 4 modülün operasyon listesi.
